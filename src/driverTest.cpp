#include <iostream>
#include <string>
#include <vector>

#include "../src/AudioTree.hpp"

int main(){
  AudioTree ItalianVocals;

  // NOTE: AUDIO MUST BE SAVED AS 16-BIT PCM WAV
  // lower-case shortened name: TuttoQuelloCheHo didn't work but tqch did???
  // std::string fname = "dopotutto"; // still too many letters...
  std::string fname = "perfect";
  // TEST 1: import data from vocal
    // std::vector<double> em = ItalianVocals.importData(fname);

  std::string nonVoc = "Bass";
  // nonVoc = fname;

  // TEST 2: insert tree node for vocals
    ItalianVocals.insertVocal(fname, 1, 44100);

  // TEST 3: search for existing node by filename
    ItalianVocals.searchVocal(fname);
  // 3B: search for non existant node
    // ItalianVocals.searchVocal("vicolo_cieco");

  // TEST 4: insert new non vocal LL nodes
    ItalianVocals.insertNonVocal(nonVoc,44100,fname);
  // 4A-C testing searches w/ updated tree
    // ItalianVocals.searchNonVocal(nonVoc, fname);
    // ItalianVocals.searchNonVocal("ThisBassDoesntExist", fname);
    // ItalianVocals.searchNonVocal("ThisBassDoesntExist", "vicolo_cieco");

  // 4D second vocal, same non vocals
    std::string fn2 = "dopo";
    std::cout << "Inserting Vocal 2... " << std::endl;
    ItalianVocals.insertVocal(fn2, 1, 44100);
    ItalianVocals.insertNonVocal(nonVoc,44100,fn2);
    ItalianVocals.searchNonVocal(nonVoc, fn2);
    ItalianVocals.searchNonVocal(nonVoc, fname);

/* UNCOMMENT BLOCK WHEN DONE TESTING ABOVE...
    std::vector<std::string> mixerNonVocals;
    mixerNonVocals.push_back(nonVoc);

  // TEST 5: initiallize mixer structure
    ItalianVocals.initMixer(fname,mixerNonVocals);
    ItalianVocals.mixAudio();

    // ItalianVocals.updateVocalFilter("sangria");
    ItalianVocals.updateVocalFilter();
    ItalianVocals.exportData();
*/

  return 0;
}
