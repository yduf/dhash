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


struct dim_t {
  int w;
  int h;
  int rot90;
};

// load & reduce image to w,h and convert to b&W
VImage reduce( VImage in,
               dim_t k  
    ) {

    int h = k.h, w = k.w;
    if( k.rot90) swap( h, w);

    VImage reduced = in
                    .reducev( in.height()/ h)
                    .reduceh( in.width() / w)
                    .colourspace(VIPS_INTERPRETATION_sRGB, 
                                 VImage::option ()
                                    ->set ("source_space", VIPS_INTERPRETATION_B_W)
                                  )
                    ;

    if( k.rot90) reduced = reduced.rot90();

    return reduced;
}

uint64_t dhash( VImage hash ) {
    auto w = hash.width();
    auto h = hash.height();
    //cout << "w: " << w << " h: " << h << "band: " << hash.bands() << "\n";
               
    auto* p = (uint8_t*) hash[0].data();          // this trigger the pipeline and take 20ms

    uint64_t hash_value = 0;

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            hash_value <<= 1;
            //hash_value |= hash(i, j)[0] > 0 ? 1 : 0;    // 1001101100111001101011010110000010011000011000110000111
            hash_value |= *p++ > 0 ? 1 : 0;               // 1001101100111001101011010110000010011000011000110000111
        }
    }

    return hash_value;
}



dim_t key( VImage from) {
  dim_t res = {};

  double r = (double) from.width() / from.height();

  if( r < 1.0) {
    res.rot90 = true;
    r = 1./r;
  }

  // find closest form factor
  std::vector<int> v = { 10, 13, 17};
  int closest = 17;

  // return matching hash mask
  std::map<int, dim_t> m { {10, { 8, 8, 0}},
                           {13, { 9, 7, 0}},
                           {17, { 10, 6, 0}},
                       };
  auto k = m[closest];
  res.w = k.w + 1;
  res.h = k.h;

  return res;

}


int
main (int argc, char **argv)
{ 
  bool debug = true;

  if (VIPS_INIT (argv[0])) 
    vips_error_exit (NULL);

  if (argc < 3+1)
    vips_error_exit ("usage: %s 0/1 w h file* ", argv[0]);

  int i = 1;
  int rot = atoi (argv[i++]);
  int width = atoi (argv[i++]);
  int height = atoi (argv[i++]);

  for( const char* name=argv[i++];i<=argc;name=argv[i++]) {

    VImage in = VImage::new_from_file (name );
    auto k = key( in);
    int h = k.h, w = k.w;

    VImage reduced = reduce( in, k );
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
        {"rot90", k.rot90},
        {"width", diff.width() },
        {"height", diff.height() },
        {"file", name}
    };

    std::cout << j << "\n";

    std::cout << std::bitset<64>( hash) << "\n";
  }


  vips_shutdown ();

  return 0;
}