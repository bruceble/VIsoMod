#ifndef AUDIOTREE_HPP
#define AUDIOTREE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>

#include "../../Mu.S.I.C.A./PhaseI/audioView/src/AudioView.hpp"

/*  Linked List node structure that will be stored at each node in the BST
    LL nodes are sorted by nonVocalAmplitude.size() in increasing order
    tie breaker goes to existing LL node: if [curr...]->size() == [curr->next...]->size()
      i.e. traverse until curr->next...-> size > insertingNode
        insert insertingNode beteween curr and curr->next
*/
struct LLNonVocalNode{
    std::string filename;
    int samplingRate;

    std::vector<double> nonVocalAmplitude;
    std::vector<double> isolatedVocalAmplitude;
    std::vector<double> mixedAmplitude;
    std::vector<std::vector<int>> spectralData;
    std::vector<std::vector<int>> vocalSpectralData;

    LLNonVocalNode* next = NULL;
};

/* Node struct that will be stored in the AudioTree BST
   Tree nodes are sorted alphabetically by VocalTreeNode->filename
*/
struct VocalTreeNode{
    LLNonVocalNode* head = NULL;
    std::string filename;
    int languageID;
    int samplingRate;

    std::vector<double> vocalAmplitude;
    std::vector<std::vector<int>> spectralData;

    VocalTreeNode* parent = NULL;
    VocalTreeNode* left = NULL;
    VocalTreeNode* right = NULL;

    std::string getLanguage(){
      std::vector<std::string> lang = {"English","Italian","Spanish","French"};
      return lang.at(languageID);
    }
};

struct mixerTracks{
    VocalTreeNode* mixerVocalTrack;
    std::vector<LLNonVocalNode*> mixerNonVocalTracks;
};

struct dataFilters{
    std::vector<int> languageID; // check size of each vector
    std::vector<int> sizeBounds; // two elments {min, max} set one zero if only want other
    std::vector<mixerTracks> includeVocals; // vec of vocals to include and which of their NonVocals LL's to include
};


class AudioTree{
    private:
      VocalTreeNode* root;
      mixerTracks init;
      mixerTracks tempMixer;
      dataFilters* filt;
    public:
      AudioTree();
      ~AudioTree();
      void showTree();
      void showFilt();
      void showAudioMixes();
      void inorderTraversal();

      void insertVocal(std::string filename, int languageID, int samplingRate);
      void insertNonVocal(std::string filename, int samplingRate, std::string vocalFilename); // use searchVocal() to get node ptr arg

      void removeVocal(VocalTreeNode* treeNodeRemove); // use searchVocal() to get node ptr arg
      void removeNonVocal(VocalTreeNode* treeNodeRemove, LLNonVocalNode* listNodeRemove); // use both search methods to get the two node args

      VocalTreeNode* searchVocal(std::string filename);
      LLNonVocalNode* searchNonVocal(std::string filename, std::string vocalFilename);

      // operational stuff
      std::vector<double> importData(std::string filename); // call in both insert methods

      void initMixer(std::string vocalFilename, std::vector<std::string> nonVocalFilenames);

      void mixNonVocals(); // mixes 2+ Non Vocal Tracks
                                                                        // edge case for duplicates? make sure filename phrase is unique between all the track full names
      void mixAudio(); // mixes audio, performs STFT, stores in LLNode Spectral Data

      void updateVocalFilter();
      void updateVocalFilter(std::string filename);

      void exportData();
      void exportData(int get_spectrographs); // input any int to save spectraph png/avi
      void exportData(dataFilters* filters); // use to replace updateVocalFilter...
};

#endif
