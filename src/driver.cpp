#include <iostream>
#include <string>
#include <vector>

#include "../src/AudioTree.hpp"

int main(){
  AudioTree ItalianVocals;

  // NOTE: AUDIO HAS BE SAVED AS 16-BIT PCM WAV
  // NOTE: NAMING FORMAT: TOTAL 4 CHARACTERS, FIRST CHAR IS CAP FOR NON VOCAL
  std::vector<std::string> vocals_filenames_it;
  vocals_filenames_it.push_back("agan");
  vocals_filenames_it.push_back("amor");
  vocals_filenames_it.push_back("crim");
  vocals_filenames_it.push_back("dopo");
  vocals_filenames_it.push_back("dtda");
  vocals_filenames_it.push_back("lotr");
  vocals_filenames_it.push_back("nyor");
  vocals_filenames_it.push_back("perf");
  vocals_filenames_it.push_back("tqch");

  std::vector<std::string> non_vocals_filenames;
  non_vocals_filenames.push_back("Bass");
  non_vocals_filenames.push_back("Gtar");
  non_vocals_filenames.push_back("Mix1");
  non_vocals_filenames.push_back("Mix2");
  non_vocals_filenames.push_back("Viol");


  std::string fname, nonVoc;
  for(int i = 0; i<vocals_filenames_it.size(); i++){
    fname = vocals_filenames_it.at(i);
    std::cout << "Driver Inserting: " << fname << ".wav" << std::endl;
    ItalianVocals.insertVocal(fname, 1, 44100);
    for(int j = 0; j<non_vocals_filenames.size(); j++){
      nonVoc = non_vocals_filenames.at(j);
      ItalianVocals.insertNonVocal(nonVoc,44100,fname);
    }
    ItalianVocals.initMixer(fname,non_vocals_filenames);
    ItalianVocals.mixAudio();
    ItalianVocals.updateVocalFilter();
  }

  ItalianVocals.showTree();
  ItalianVocals.showFilt();
  ItalianVocals.exportData();


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
