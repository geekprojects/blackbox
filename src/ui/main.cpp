
#include "blackbox.h"
#include "importers/volanta.h"

using namespace std;

int main(int argc, char** argv)
{
#if 0
    DataStore dataStore;

    dataStore.init("/Users/ian/projects/blackbox/test/blackbox.db");

    VolantaImporter importer(&dataStore);
    importer.import("/Users/ian/Downloads/volanta-export");
#else
    BlackBoxUI blackBoxUI(argc, argv);
    return blackBoxUI.run();
#endif
}
