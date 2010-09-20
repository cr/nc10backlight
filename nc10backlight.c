#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <pciaccess.h>

int isNc10( struct pci_device * dev ) {
	char * p;
	uint16_t id;
	if( dev == NULL ) return 0;
	p = (char *) pci_device_get_device_name( dev );
	if( p == NULL ) return 0;
	if( strcmp( p, "Mobile 945GM/GMS/GME, 943/940GML Express Integrated Graphics Controller" ) )
		return 0;
	p = (char *) pci_device_get_subvendor_name( dev );
	if( p == NULL ) return 0;
	if( strcmp( p, "Samsung Electronics Co Ltd" ) ) return 0;
	if( pci_device_cfg_read_u16( dev, &id, 0x2e ) ) return 0;
	if( id != 0xca00 ) return 0;
	return 1;
}

uint8_t getBrightness( struct pci_device * dev ) {
	uint8_t val;
	pci_device_cfg_read_u8( dev, &val, 0xf4 );
	return val;
}

void setBrightness( struct pci_device * dev, uint8_t val) {
	pci_device_cfg_write_u8( dev, val, 0xf4 );
}

void usage() {
	fputs( "nc10backlight 0.1 by cr\n", stderr );
	fputs( "usage: nc10backlight [mode [level]]\n", stderr );
	fputs( "       where mode can be -get|-set|=|-inc|+|-dec|-\n", stderr );
	fputs( "       default mode is -get\n", stderr );
}

int main( int argc, char ** argv ) {

	struct pci_device * dev;
	uint8_t val;
	int newval;
	int mode;
	int argp;

	setuid(0);
	if( pci_system_init() ) {
		fprintf( stderr, "ERROR: PCI bus init failed. No root?\n" );
		exit( 1 );
	}

	dev = pci_device_find_by_slot( 0, 0, 2, 1 );
	if( !isNc10( dev ) ) {
		fprintf( stderr, "ERROR: This device does not look not like a Samsung NC10.\n" );
		exit( 1 );
	}

	mode = 0;
	argp = 1;
	switch( argc ) {
	case 1:
		mode = 0;
		break;
	case 2:
		if( !strcmp( "-get", argv[argp] ) ) {
			mode=0;
			argp++;
		} else {
			usage();
			exit( 1 );
		}
		break;
	case 3:
		if( !strcmp( "-set", argv[argp] ) || !strcmp( "=", argv[argp] ) ) {
			mode = 1;
			argp++;
		} else if( !strcmp( "-inc", argv[argp] ) || !strcmp( "+", argv[argp] ) ) {
			mode = 2;
			argp++;
		} else if( !strcmp( "-dec", argv[argp] ) || !strcmp( "-", argv[argp] ) ) {
			mode = 3;
			argp++;
		} else {
			usage();
			exit( 1 );
		}
		break;
	default:
		usage();
		exit( 1 );
	}

	switch( mode ) {
	case 0: /* get */
		val = getBrightness( dev );
		printf("%d\n", val);
		break;
	case 1: /* set */
		val = (uint8_t) atoi( argv[argp] );
		setBrightness( dev, val );
		break;
	case 2: /* inc */
		val = getBrightness( dev );
		newval = val + atoi( argv[argp] );
		if( newval > 255 ) newval = 255;
		setBrightness( dev, (uint8_t) newval );
		break;
	case 3: /* dec */
		val = getBrightness( dev );
		newval = val - atoi( argv[argp] );
		if( newval < 0 ) newval = 0;
		setBrightness( dev, (uint8_t) newval );
		break;
	}

	pci_system_cleanup();
	return 0;
}

