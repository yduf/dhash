/* compile with:
 *      g++ -g -Wall example.cc `pkg-config vips-cpp --cflags --libs`
 */

#include <cstdint>
#include <bitset>
#include <iostream>

#include <vips/vips8>
#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;
using namespace vips;
using namespace std;


// load & reduce image to w,h and convert to b&W
VImage reduce( const char* name,
               int w, int h  
    ) {
    VImage in = VImage::new_from_file (name,
                                       VImage::option ()
                                       //   ->set ("access", VIPS_ACCESS_SEQUENTIAL)
                                      );

    VImage reduced = in
                    .reducev( in.height()/ h)
                    .reduceh( in.width() / w)
                    .colourspace(VIPS_INTERPRETATION_sRGB, 
                                 VImage::option ()
                                    ->set ("source_space", VIPS_INTERPRETATION_B_W)
                                  )
                    ;

    return reduced;
}

uint64_t dhash( VImage hash ) {
    auto w = hash.width();
    auto h = hash.height();
    cout << "w: " << w << " h: " << h << "band: " << hash.bands() << "\n";

    size_t size;
    auto* data = (uint8_t*) hash[0].write_to_memory( &size);
    uint64_t hash_value = 0;
    auto* p = data;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            hash_value <<= 1;
            //hash_value |= hash(i, j)[0] > 0 ? 1 : 0;    // 1001101100111001101011010110000010011000011000110000111
            hash_value |= *p++ > 0 ? 1 : 0;               // 1001101100111001101011010110000010011000011000110000111
        }
    }

    g_free( data);

    return hash_value;
}

int
main (int argc, char **argv)
{ 
  bool debug = false;

  if (VIPS_INIT (argv[0])) 
    vips_error_exit (NULL);

  if (argc < 3+1)
    vips_error_exit ("usage: %s 0/1 w h file* ", argv[0]);

  int i = 1;
  int rot = atoi (argv[i++]);
  int width = atoi (argv[i++]);
  int height = atoi (argv[i++]);

  for( const char* name=argv[i++];i<=argc;name=argv[i++]) {
    int h = 11, w = 6;

    VImage reduced = reduce( name, w, h );
    if( debug) reduced.write_to_file ("reduced.pgm");
    
    VImage A = reduced.crop( 0, 0, w - 1, h );
    VImage B = reduced.crop( 1, 0, w - 1, h );

    if( debug) A.write_to_file ("A.pgm");
    if( debug) B.write_to_file ("B.pgm");

    VImage diff = A < B; 
    if( debug) diff.write_to_file ("diff.pgm");

    auto hash = dhash( diff );
    json j = {
        {"hash", hash},
        {"rot", rot},
        {"width", diff.width() },
        {"height", diff.height() },
        {"file", name}
    };

    std::cout << j << "\n";

    std::cout << std::bitset<55>( hash) << "\n";
  }


  vips_shutdown ();

  return 0;
}