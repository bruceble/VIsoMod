#include <iostream>
#include <string>
#include <vector>

#include "../src/AudioTree.hpp"

AudioTree::AudioTree(){
  root = NULL;
  // init = new mixerTracks;
  // tempMixer = init;
  filt = new dataFilters;
  std::vector<int> languageID; // check size of each vector
  std::vector<int> sizeBounds; // two elments {min, max} set one zero if only want other
  std::vector<mixerTracks> includeVocals; // vec of vocals to include and which of their NonVocals LL's to include
  filt->languageID = languageID;
  filt->sizeBounds = sizeBounds;
  filt->includeVocals = includeVocals;
}

AudioTree::~AudioTree(){
  //
  // delete init;
  // init = NULL;

  delete filt;
  filt = NULL;
}

// TODO : ADD ERROR LOG FX-ALITY TO TRACK IF AN AUDIO FILE DOESN'T LOAD

void showTreeHelper(VocalTreeNode * root) {
		if (root == NULL) {
			return;
		}

		showTreeHelper(root->left);

    LLNonVocalNode* curr = root->head;
    if(curr != NULL){
		    std::cout << "Vocal Node: " << root->filename << ".wav" << std::endl;
        std::cout << "  Language: " << root->getLanguage() << std::endl;
    }
    std::cout << "  Non Vocal Node(s): \n";
		while(curr != NULL){
		    std::cout << "    >> " << curr->filename << ".wav" << std::endl;
        if(curr->isolatedVocalAmplitude.size() != curr->mixedAmplitude.size()){
          std::cout << "        >> " << curr->isolatedVocalAmplitude.size() << " & " << curr->mixedAmplitude.size() << std::endl;
        }
		    curr = curr->next;
	    }

		showTreeHelper(root->right);
}

void AudioTree::showTree(){
    showTreeHelper(root);
}

void AudioTree::showFilt(){
    std::cout << "Showing Filter: " << std::endl;
    int N = filt->includeVocals.size();
    for(int i=0;i<N;i++){
      std::cout << i << ": " << filt->includeVocals.at(i).mixerVocalTrack->filename << std::endl;
      for(int j=0;j<filt->includeVocals.at(i).mixerNonVocalTracks.size();j++){
        std::cout << "     " << filt->includeVocals.at(i).mixerNonVocalTracks.at(j)->filename << std::endl;
      }
    }
}

std::vector<double> AudioTree::importData(std::string filename){

  std::vector<double> data;
  FILE *pipe_in;
  char phrase_in[50];
  int16_t sample;
  int count;
  double amp_i;

  // audio file  must be in ../src/res/audio/
  sprintf(phrase_in, "ffmpeg -i ../src/res/audio/%s.wav -f s16le -ac 1 -", filename.c_str()); // using -ac 2 adds reflected data...
  pipe_in  = popen(phrase_in, "r"); // open pipe, "r" signifies read

  while(1){
      count = fread(&sample, 2, 1, pipe_in); // read one 2-byte sample
      data.push_back(sample);
      if (count != 1) break;
  }

  pclose(pipe_in); // close pipe

  return data;
}

void _treeNodeInsert(VocalTreeNode* curr, VocalTreeNode* newVocal){
  // helper for insertVocal(), compare strings with newVocal->filename
  // TODO: implement recursive insertion from hw
  if (curr == NULL) return;

  // move left until recursive root node is before newVocal alphabetically
  if(curr->filename > newVocal->filename) _treeNodeInsert(curr->left, newVocal);

  // move right until recursive root note is after newVocal alphabetically
  if(curr->filename < newVocal->filename) _treeNodeInsert(curr->right, newVocal);

  // determine if tree node should be inserted ->left or ->right
  if(curr->filename > newVocal->filename && curr->left == NULL){
    newVocal->parent = curr;
    curr->left = newVocal;
    return;
  }
  else if(curr->filename < newVocal->filename && curr->right == NULL){
    newVocal->parent = curr;
    curr->right = newVocal;
    return;
  }

}

void AudioTree::insertVocal(std::string filename, int languageID, int samplingRate){
  // dyn alloc
  VocalTreeNode* newVocal = new VocalTreeNode;

  // set values
  newVocal->vocalAmplitude = importData(filename);
  newVocal->filename = filename;
  newVocal->languageID = languageID;
  newVocal->samplingRate = samplingRate;

  // EDGE CASE: EMPTY TREE
  if(root == NULL){
    root = newVocal;
    return;
  }

  // use recursion to insert newVocal into tree
  _treeNodeInsert(root, newVocal);
}

VocalTreeNode* searchVocalHelper(VocalTreeNode* curr, std::string filename){
    if(curr == NULL) return curr;
    else if(curr->filename == filename) return curr;
    else if(curr->filename > filename) return searchVocalHelper(curr->left, filename);
    else return searchVocalHelper(curr->right, filename);
}

VocalTreeNode* AudioTree::searchVocal(std::string filename){
    VocalTreeNode* foundNode = searchVocalHelper(root, filename);
    if(foundNode == NULL) std::cout << "Did not find " << filename << ".wav" << std::endl;
    else std::cout << "Found " << foundNode->filename << ".wav" <<  std::endl;
    return foundNode;
}

LLNonVocalNode* searchNonVocalHelper(VocalTreeNode* curr, VocalTreeNode* selectedVocalNode, std::string filename){
    if (curr == NULL) return NULL;
    else if(curr->filename == selectedVocalNode->filename){
      LLNonVocalNode* currLL = curr->head;
      while(currLL != NULL){
        if(currLL->filename == filename) return currLL;
        currLL = currLL->next;
      }
    }
    else if(curr->filename > selectedVocalNode->filename){
      return searchNonVocalHelper(curr->left, selectedVocalNode, filename);
    }
    else return searchNonVocalHelper(curr->right, selectedVocalNode, filename);
}

/* SCANS ENTIRE TREE, RETURNS NODE IF NON VOCAL FILE WAS ALREADY IMPORTED */
LLNonVocalNode* scanNonVocalHelper(VocalTreeNode* curr, std::string filename){
    if (curr == NULL) return NULL;
    scanNonVocalHelper(curr->left, filename);

    LLNonVocalNode* currLL = curr->head;
    while(currLL != NULL){
      if(currLL->filename == filename) return currLL;
      currLL = currLL->next;
    }

    scanNonVocalHelper(curr->right, filename);
    return NULL;
}

LLNonVocalNode* AudioTree::searchNonVocal(std::string filename, std::string vocalFilename){
    std::cout << "Searching for " << filename << ".wav" << " with vocal: " << vocalFilename << ".wav" << std::endl;
    VocalTreeNode* selectedVocalNode = searchVocal(vocalFilename);
    LLNonVocalNode* foundNode = searchNonVocalHelper(root, selectedVocalNode, filename);
    if(foundNode == NULL) std::cout << "Did not find " << filename << ".wav" << std::endl;
    else std::cout << "Found " << foundNode->filename << ".wav" <<  std::endl;
    return foundNode;
}

void _LLNodeInsert(LLNonVocalNode* newNonVocal, VocalTreeNode* selectedVocalNode){
  /* ASSUME NO DUPLICATES OF NON VOCAL NODES WITHIN A TREE NODE*/

  // EDGE CASE: EMPTY LL
  if(selectedVocalNode->head == NULL){
    selectedVocalNode->head = newNonVocal;
    return;
  }

  LLNonVocalNode* curr = selectedVocalNode->head;

  // EDGE CASE: FIRST NODE IS LARGER THAN NEW NODE IN SIZE
  if(curr->nonVocalAmplitude.size() > newNonVocal->nonVocalAmplitude.size()){
    newNonVocal->next = curr;
    selectedVocalNode->head = newNonVocal;
    return;
  }

  // EDGE CASE: FIRST NODE IS EQUAL THAN NEW NODE IN SIZE, NEW COMES FIRST ALPHABETICALLY
  if(curr->nonVocalAmplitude.size() == newNonVocal->nonVocalAmplitude.size() && newNonVocal->filename < curr->filename){
    newNonVocal->next = curr;
    selectedVocalNode->head = newNonVocal;
    return;
  }

  // INSERTION WHILE LOOP
  while(curr->next != NULL){
    // CASE: NEW NODE IS LARGER SIZE THAT CURR AND SMALLER THAN NEXT
    if(curr->nonVocalAmplitude.size() < newNonVocal->nonVocalAmplitude.size() && curr->next->nonVocalAmplitude.size() > newNonVocal->nonVocalAmplitude.size()){
      newNonVocal->next = curr->next;
      curr->next = newNonVocal;
      return;
    }
    // CASE: NEXT AND NEW ARE SAME SIZE, AND NEW COME FIRST ALPHABETICALLY
    if(curr->next->nonVocalAmplitude.size() == newNonVocal->nonVocalAmplitude.size() && newNonVocal->filename < curr->next->filename){
      newNonVocal->next = curr->next;
      curr->next = newNonVocal;
      return;
    }

    curr = curr->next;
  }

  // EDGE CASE: ALL NODES COME BEFORE NEW NODE IN ALPHABET
  curr->next = newNonVocal;
  return;
}

void AudioTree::insertNonVocal(std::string filename, int samplingRate, std::string vocalFilename){
  //
  // assume that Vocal node (selectedNode) already exists...
    // this program will only add NonVocal LL nodes to existing tree nodes
  VocalTreeNode* selectedVocalNode = searchVocal(vocalFilename);
  if(selectedVocalNode == NULL){
    std::cout << "Could not find vocal: "<< vocalFilename << std::endl;
    return;
  }
  LLNonVocalNode* newNonVocal = new LLNonVocalNode;
  LLNonVocalNode* existingNode = scanNonVocalHelper(root, filename);
  std::vector<double> importedNonVocal;
  if(existingNode == NULL){
    importedNonVocal = importData(filename);
  }
  else{
    std::cout << filename << " was already imported, copying data... " << std::endl;
    importedNonVocal = existingNode->nonVocalAmplitude;
  }

  // set values
  newNonVocal->nonVocalAmplitude = importedNonVocal;
  newNonVocal->filename = filename;
  newNonVocal->samplingRate = samplingRate;

  _LLNodeInsert(newNonVocal, selectedVocalNode);
}

void AudioTree::initMixer(std::string vocalFilename, std::vector<std::string> nonVocalFilenames){

    std::cout << "\nInitializing mixer for " << vocalFilename << " with the following non vocals..." << std::endl;

    init.mixerVocalTrack = searchVocalHelper(root, vocalFilename);

    int N = nonVocalFilenames.size();
    for(int i = 0; i<N; i++){
      LLNonVocalNode* addLLNode = searchNonVocalHelper(root, init.mixerVocalTrack, nonVocalFilenames.at(i));
      init.mixerNonVocalTracks.push_back(addLLNode);
      std::cout << init.mixerNonVocalTracks.at(i)->filename << "--" << init.mixerVocalTrack->filename << std::endl;
    }

    std::cout << "Complete." << std::endl;

}

void AudioTree::mixNonVocals(){
  mixerTracks* mixerAudio = &init;
  int N = mixerAudio->mixerNonVocalTracks.size();
  int sizeMin = 1073741824; // 2^30 (max int size is 2^31)
  std::string mixedName;

  if(N==0){
    std::cout << "No Non Vocal Tracks were found: aborting mix" << std::endl;
    return;
  }


  for(int i=0;i<N;i++){
    for(int j=0;j<N;j++){
      // nested-for to see if there are any filename repeats or sampling rate mismatches
      if(i != j && mixerAudio->mixerNonVocalTracks.at(i)->filename == mixerAudio->mixerNonVocalTracks.at(j)->filename){
        std::cout << "Duplicate filename found: aborting mix" << std::endl;
        return;
      }
      // sampling rate of the NonVocals don't have to match, the only SR that matters is the vocal part
      // if(i != j && mixerAudio->mixerNonVocalTracks->at(i)->samplingRate != mixerAudio->mixerNonVocalTracks->at(j)->samplingRate){
      //   std::cout << "Sampling Rate Mismatch: aborting mix" << std::endl;
      //   return;
      // }
    }
    // update sizeMin to determine smallest nonVocalAmplitude data vector
    if(mixerAudio->mixerNonVocalTracks.at(i)->nonVocalAmplitude.size() < sizeMin){
      sizeMin = mixerAudio->mixerNonVocalTracks.at(i)->nonVocalAmplitude.size();
    }
    // combine filenames
    mixedName +=mixerAudio->mixerNonVocalTracks.at(i)->filename + "_";
  }

  std::vector<double> mixedData;
  double combinedData = 0;
  for(int i=0;i<sizeMin;i++){
    for(int j=0;j<N;j++){
      combinedData += mixerAudio->mixerNonVocalTracks.at(j)->nonVocalAmplitude.at(i);
    }
    mixedData.push_back( combinedData );
    combinedData = 0;
  }

  VocalTreeNode* selectedVocalNode = mixerAudio->mixerVocalTrack;

  LLNonVocalNode* newNonVocal = new LLNonVocalNode;

  // set values
  newNonVocal->nonVocalAmplitude = mixedData;
  newNonVocal->filename = mixedName;
  newNonVocal->samplingRate = mixerAudio->mixerNonVocalTracks.at(0)->samplingRate;

  _LLNodeInsert(newNonVocal, selectedVocalNode);

}

void AudioTree::mixAudio(){
    mixerTracks* mixerAudio = &init;
    std::cout << "\nMixing " << mixerAudio->mixerVocalTrack->filename << " with the following non vocals to ->mixedAmplitude ..." << std::endl;
    int N = mixerAudio->mixerNonVocalTracks.size();
    int L = mixerAudio->mixerVocalTrack->vocalAmplitude.size();
    int len;
    double combinedData = 0;

    for(int i=0;i<N;i++){
      std::cout  << "    >> " << mixerAudio->mixerNonVocalTracks.at(i)->filename <<std::endl;
      // CASE: NON-VOCAL TRACK IS LONGER THAN VOCAL TRACK
      if(L<mixerAudio->mixerNonVocalTracks.at(i)->nonVocalAmplitude.size()){
        len = L;
      }
      // CASE: VOCAL TRACK IS LONGER THAT NON VOCAL
      else{
        len = mixerAudio->mixerNonVocalTracks.at(i)->nonVocalAmplitude.size();
      }
      // ADD VOCALS + NON VOCALS, PUSH TO mixedAmplitude and isolatedVocalAmplitude
      for(int j=0;j<len;j++){
        combinedData += mixerAudio->mixerVocalTrack->vocalAmplitude.at(j);

        // pushing just the vocal data ( so that length of isolated audio matches the length of mixed audio)
        mixerAudio->mixerNonVocalTracks.at(i)->isolatedVocalAmplitude.push_back(combinedData);

        // add on the non vocal track
        combinedData += mixerAudio->mixerNonVocalTracks.at(i)->nonVocalAmplitude.at(j);
        mixerAudio->mixerNonVocalTracks.at(i)->mixedAmplitude.push_back(combinedData);
        combinedData = 0; // reset
      }
    }

    init = *mixerAudio;
    std::cout << "Complete" << std::endl;
}

void AudioTree::updateVocalFilter(std::string filename){
  // VocalTreeNode* vocalFilter = searchVocalHelper(root, filename);
  // filt->includeVocals.push_back(vocalFilter);
}

void AudioTree::updateVocalFilter(){
  filt->includeVocals.push_back(init);
  std::cout << init.mixerVocalTrack->filename << ": ";
  init = tempMixer;
  showFilt();

}

void _titleRowOnce(int rows, int cols, std::vector<std::vector<float>> vec2D, bool& titled, std::string csv_string){
  if(!titled){
    std::ofstream outputCSV;
    outputCSV.open(csv_string, std::ios_base::app); // append

    for(int c=0; c<cols; c++){
      for(int r=0; r<rows; r++){
        outputCSV << r << "x" << c << ",";
      }
    }
    outputCSV << "\n";
    outputCSV.close();
    titled = true;
  }

}

void _formatCSV(std::vector<std::vector<float>> vec2D, bool& titled, std::string csv_string){
  int rows = vec2D.size();
  int cols = vec2D[0].size();
  int ct = 0;
  int FRAMES = 25; // number of STFT Frames used for temporal context
  _titleRowOnce(rows, 25, vec2D, titled, csv_string);

  std::ofstream outputCSV;
  outputCSV.open(csv_string, std::ios_base::app); // append

  for(int c=0; c<cols; c++){
    for(int r=0; r<rows; r++){
      outputCSV << vec2D[r][c] << ",";
    }
    if( (c+1)%FRAMES == 0) {
      outputCSV << "\n"; // if reach the last FRAME of temporal context carriage return to next csv row
      ct++;
    }
    if( (cols-c)<FRAMES) break; // breaks if cols remaining is less that the desired Frame size
  }
  outputCSV << "\n";

  outputCSV.close();
  std::cout << "Num of added csv rows:" << ct << std::endl;
}

void AudioTree::exportData(){ // removed (dataFilters* filters)
  dataFilters* filters = filt;
  int V = filters->includeVocals.size(); // size = #/vocals. for ea vocal, need to det. #/non-vocals
  if(V==0){
      std::cout << "No vocal filters, checking languages and sizes..." << std::endl;
      int L = filters->languageID.size();
      int S = filters->sizeBounds.size();

      std::vector<mixerTracks> filterIncludeVocals;

      // TODO create mixerTrack* vec using L and S Filters (since vector provided is empty)

      filters->includeVocals = filterIncludeVocals;
      V = filters->includeVocals.size();
      if(V==0){
        std::cout << "Filters too specific, no Vocals Found: aborting" << std::endl;
        return;
      }
  }

  AudioView AView;
  int NV;
  int dim1, dim2;
  int ct1, ct2;
  std::vector<double> currMix;
  std::vector<std::vector<float>> spectralData;
  bool isTitled = false;
  bool isTitledVerif = false;
  for(int i=0; i<V; i++){
    std::cout << filters->includeVocals.at(i).mixerVocalTrack->filename << std::endl;
  }

  for(int i=0;i<V;i++){
    NV = filters->includeVocals.at(i).mixerNonVocalTracks.size();
    std::cout << "\nExporting [" << i+1 << "/" << V << "] " << filters->includeVocals.at(i).mixerVocalTrack->filename << " with the following non vocals ..." << std::endl;
    for(int j=0;j<NV;j++){
      std::cout  << "    >> " << filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->filename <<std::endl;
      currMix = filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->mixedAmplitude;
      AView.calculateSpectrograph(256, currMix);
      spectralData = AView.getSpectralData();
      filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->spectralData = spectralData;

      std::string fnNonVocal = filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->filename;
      std::string fnVocal = filters->includeVocals.at(i).mixerVocalTrack->filename;

      // AView.saveSpectrograph(fnNonVocal); // skipped to save space, uncomment for viewing purposes
      // AView.animateSpectrograph(fnNonVocal); // skipped to save space, uncomment for viewing purposes

      std::string writeName = std::string(1,fnVocal[0]) + fnVocal[1] + "_" + fnNonVocal[0] + fnNonVocal[1];

      dim1 = spectralData.size()*spectralData[0].size();
      ct1 = currMix.size();

      _formatCSV(spectralData, isTitled,"train.csv");
      std::cout << "        Added to train.csv..." << ct1 << " & " << dim1 << std::endl;
      // AView.write(currMix, writeName); // skipped to save space, uncomment for viewing purposes
      AView.resetSpectrograph();
      currMix.clear();
      spectralData.clear();

      std::cout << "        Spect reset successful..." << std::endl;

      ///////////////////////// Repeat spectrogram process for vocals only to get verif data
      currMix = filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->isolatedVocalAmplitude;
      std::cout << "        Spect calculation started..." << std::endl;
      AView.calculateSpectrograph(256, currMix);
      std::cout << "        Spect calculation successful..." << std::endl;
      spectralData = AView.getSpectralData();
      filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->vocalSpectralData = spectralData;

      // AView.saveSpectrograph(fnVocal); // skipped to save space, uncomment for viewing purposes
      // AView.animateSpectrograph(fnVocal); // skipped to save space, uncomment for viewing purposes

      dim2 = spectralData.size()*spectralData[0].size();
      ct2 = currMix.size();

      _formatCSV(spectralData, isTitledVerif,"verif.csv");
      std::cout << "        Added to verif.csv..." << ct2 << " & " << dim2 << std::endl;
      AView.resetSpectrograph();
      currMix.clear();
      spectralData.clear();

      std::cout << "        Spect reset successful..." << std::endl;
    }
  }

  // TODO reformat and write to .csv

}

void AudioTree::exportData(int get_spectrographs){ // removed (dataFilters* filters)
  dataFilters* filters = filt;
  int V = filters->includeVocals.size(); // size = #/vocals. for ea vocal, need to det. #/non-vocals
  if(V==0){
      int L = filters->languageID.size();
      int S = filters->sizeBounds.size();

      std::vector<mixerTracks> filterIncludeVocals;

      // TODO create mixerTrack* vec using L and S Filters (since vector provided is empty)

      filters->includeVocals = filterIncludeVocals;
      V = filters->includeVocals.size();
      if(V==0){
        std::cout << "Filters too specific, no Vocals Found: aborting" << std::endl;
        return;
      }
  }

  AudioView AView;
  int NV;
  std::vector<double> currMix;
  std::vector<std::vector<float>> spectralData;
  bool isTitled = false;

  for(int i=0;i<V;i++){
    NV = filters->includeVocals.at(i).mixerNonVocalTracks.size();
    for(int j=0;j<NV;j++){
      currMix = filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->mixedAmplitude;
      AView.calculateSpectrograph(256, currMix);
      spectralData = AView.getSpectralData();
      filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->spectralData = spectralData;

      std::string fnNonVocal = filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->filename;
      std::string fnVocal = filters->includeVocals.at(i).mixerVocalTrack->filename;

      AView.saveSpectrograph(fnNonVocal); // skip to save space, uncomment for viewing purposes
      AView.animateSpectrograph(fnNonVocal); // skip to save space, uncomment for viewing purposes

      std::string writeName = std::string(1,fnVocal[0]) + fnVocal[1] + "_" + fnNonVocal[0] + fnNonVocal[1];

      _formatCSV(spectralData, isTitled,"train.csv");
      // AView.write(currMix, writeName); // skipped to save space, uncomment for viewing purposes
      AView.resetSpectrograph();

      ///////////////////////// Repeat spectrogram process for vocals only to get verif data
      currMix = filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->isolatedVocalAmplitude;
      AView.calculateSpectrograph(256, currMix);
      spectralData = AView.getSpectralData();
      filters->includeVocals.at(i).mixerNonVocalTracks.at(j)->vocalSpectralData = spectralData;

      AView.saveSpectrograph(fnVocal); // skip to save space, uncomment for viewing purposes
      AView.animateSpectrograph(fnVocal); // skip to save space, uncomment for viewing purposes


      _formatCSV(spectralData, isTitled,"verif.csv");
      AView.resetSpectrograph();
    }
  }


}
