#include <iostream>
#include "Octet.h"
#include "EncodedBerObject.h"
#include "DecodedBerObject.h"
#include "Constants.h"
#include <sstream>
#include <iomanip>

using namespace BER;

std::ostream &operator<<(std::ostream &os, const EncodedBerObject &str) {
    for (int i = 0; i < 2; ++i) {
        os << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) str[i] << ' ';
    }
    if (str[1].SubBits<7,7>() == 1) {
        for (int i = 0; i < str[1].SubBits<6,0>(); ++i) {
            os << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) str[i] << ' ';
        }
    }
    os << '\n';
    for (int i = 2; i < str.size(); ++i) {
        os << std::hex << std::setw(2) << std::setfill('0')  << (unsigned short) str[i];
    }
    return os;
}

int main() {
    auto encoded = Encode(FromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse egestas posuere tellus, sed egestas mauris efficitur quis. Fusce convallis, libero nec vestibulum scelerisque, arcu nibh ullamcorper ex, ullamcorper blandit ex odio sed augue. Nulla quis ultrices justo. Duis tempus velit a augue posuere rutrum. Nam lacinia lacus eu felis accumsan scelerisque. Nulla non sollicitudin dui. Curabitur a molestie est, ac euismod tortor. Sed egestas, dui ac aliquam tempor, sapien tortor vestibulum orci, non sagittis ipsum purus eget ipsum. Suspendisse commodo imperdiet imperdiet. Aenean vitae diam a nisi dignissim posuere in ac velit. Suspendisse varius odio in egestas ornare. Vivamus congue, est ut eleifend varius, erat orci tempus eros, eu dictum augue libero non ante.\n"
                                     "\n"
                                     "Suspendisse in nulla eget arcu accumsan molestie sit amet ac ligula. Nullam id mauris id justo hendrerit faucibus in at diam. Sed ipsum quam, condimentum vel ultrices sed, eleifend nec ligula. Etiam vel massa id eros maximus tristique sit amet ac elit. Integer aliquet arcu et quam scelerisque, non varius nibh maximus. Integer faucibus dolor nulla, ut condimentum massa dignissim rhoncus. Integer feugiat laoreet dui non convallis. Praesent hendrerit diam nec nulla finibus porta. Etiam suscipit, orci ac ornare ultrices, arcu ligula dictum nisi, quis dictum neque nisl quis lacus. Ut erat odio, efficitur vestibulum mauris a, pulvinar facilisis dui. In consectetur enim et aliquam laoreet. Morbi iaculis eros quam, vel posuere nisi aliquet cursus. Ut interdum urna nec vehicula sollicitudin.\n"
                                     "\n"
                                     "Nullam pharetra urna sollicitudin augue elementum faucibus. Vivamus dolor turpis, finibus non metus et, venenatis aliquet orci. Cras pretium turpis sed metus laoreet congue. Quisque molestie rutrum magna sed scelerisque. Nulla scelerisque risus a accumsan sollicitudin. Vestibulum sit amet nulla convallis, maximus risus ac, blandit erat. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Maecenas vulputate, est nec tempus venenatis, neque neque mollis dui, eget blandit lacus elit a mauris. Donec blandit leo ornare laoreet semper. Praesent suscipit sed erat sed pellentesque. Suspendisse potenti.\n"
                                     "\n"
                                     "Suspendisse ante ipsum, rhoncus non imperdiet nec, faucibus vel augue. Aliquam erat volutpat. Vivamus ultrices, lacus convallis semper blandit, nibh erat consectetur tellus, at placerat velit eros a nulla. Duis eget nunc in lorem varius tincidunt sit amet id nibh. Fusce volutpat dui at dui mattis, vehicula volutpat magna viverra. In euismod ex id turpis hendrerit rhoncus. Pellentesque at justo id lectus vehicula dapibus non sit amet elit. Fusce dictum tortor laoreet augue congue, eget sollicitudin lacus porta.\n"
                                     "\n"
                                     "Vestibulum eget ante eget orci aliquam dignissim. Aliquam sit amet hendrerit augue, in sodales mauris. Ut lacus tellus, venenatis nec augue pellentesque, varius porta sapien. Quisque fermentum varius molestie. Nam eget diam diam. In vulputate, augue id mollis accumsan, urna eros malesuada nisi, id tristique justo purus ac leo. Nulla ante ipsum, sagittis id mollis quis, aliquam sit amet tellus. Proin sollicitudin metus metus, vel sollicitudin libero viverra nec. Praesent sed velit odio. Quisque laoreet mi nec porttitor rhoncus. Donec tristique, ligula at elementum porta, justo lacus porttitor neque, sed venenatis velit nulla vitae augue. Phasellus ultricies neque mi.\n"
                                     ""));

    std::cout << encoded;

    return 0;
}
