#include "main.h"
#include "allegro5/allegro.h"
#include "allegro5/allegro_primitives.h"
#include <stdio.h>



int mustInit( bool test, const char *msg ){
	if( !test ){
		printf( "%s couldnt init", msg );
		exit( 1 );
	}
}

int main( int argc, char **argv ){
	printf( "started\n" );
	printf("%0x\t%0x\n", ALLEGRO_VERSION_INT, al_get_allegro_version());
	mustInit( al_init(), "allegro" );
	mustInit( al_init_primitives_addon(), "primitives" );
	ALLEGRO_DISPLAY *disp = al_create_display( 800, 800 );
	mustInit (disp, "display");
	al_draw_filled_circle( 200, 200, 50, al_map_rgb(255,255,255));	
	al_draw_filled_circle( 200, 200, 30, al_map_rgb(0, 0, 0));
	al_flip_display();	
	getchar();
}





