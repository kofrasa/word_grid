/*
 * File:   main.cpp
 * Author: Francis Asante <kofrasa@gmail.com>
 * License: Free for public use.
 */

#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>

#define SIZE 12
#define WORD_LIST 18

using namespace std;

typedef struct {
  string word;
  int row, col;
  bool across;
} Entry;

bool fit_word(const string,int,int,bool);
bool clean_fit(const string&);
bool force_fit(const string&);
void clean_fit_all();
void clear_grid();
void print_grid();

vector<Entry> placed; // words placed on the grid
deque<int> rem; // index of remaining words

const char EMPTY = '.';
char grid[SIZE][SIZE];
stringstream ss;

string words[WORD_LIST] = {
  "and", "any", "apex", "assume", "barn", "cat", "devil", "dip", "dive", "dumps",
  "elephant", "extra", "liar", "lioness", "mount", "oppose", "pursuit", "rap"
};

void init() {
  clear_grid();
  string w;
  int i,j,len;  
  rem.push_back(0);
  for(i=1; i< WORD_LIST; ++i) { //sort words by length
    j = i - 1;    
    rem.push_back(i); //add word index to remaining list
    len = words[i].length();
    while(j >= 0 && len > words[j].length()) {
      w = words[j];
      words[j] = words[j+1];
      words[j+1] = w;
      j--;
    }
  }
}

void clear_grid() {
  int i,j;
  for(i=0; i < SIZE; ++i) for(j=0; j < SIZE; ++j) {
    grid[i][j] = EMPTY;
  }
}

void print_grid() {
  int i,j;
  ss << endl;
  for(i=0; i < SIZE; ++i) {
    for(j=0; j < SIZE; ++j) ss<<grid[i][j];
    ss << endl;
  }
}

void fill_grid() {
  string w;
  int size;
  deque<int> idx, leftover, backup = rem;
  vector<Entry> fitted; //successfully fitted words
  bool done = false;
  char optimal[SIZE][SIZE];  
  int prev = WORD_LIST;      
  
  int limit = SIZE - words[rem.at(0)].length();
  
  for (int i=0; i<SIZE && !done; ++i) {
    for (int j=0; j<limit && !done; ++j) {                  
      if (!fit_word(words[rem.at(0)], i, j, true)) continue;
            
      rem.pop_front();
      clean_fit_all();
      size = rem.size();

      for (int k=0; k<size; ++k) {
        w = words[rem.at(k)];
        if (clean_fit(w) || force_fit(w)) continue;
        else {          
          idx.push_back(rem.at(k));
        }
      }               
      size = idx.size();
      
      if (size < prev) {
        prev = size;
        leftover = idx;
        fitted = placed;
        //TODO: must optimize using pointers if grid is large. 
        //optimization: point optimal to grid and allocate a new space for grid
        for(int i=0; i < SIZE; ++i) for(int j=0; j<SIZE; ++j) optimal[i][j] = grid[i][j];
        done = prev == 0;
      }
      
      if (!done) {
        rem = backup;
        idx.clear();
        clear_grid();
        placed.clear();        
      }
    }
  }
  rem = leftover;
  placed = fitted;
  for(int i=0; i < SIZE; ++i) for(int j=0; j<SIZE; ++j) grid[i][j] = optimal[i][j];  
}

void clean_fit_all() {
  deque<int> idx;
  for (int i =0; i<rem.size(); ++i) {
    if (!clean_fit(words[rem.at(i)])) {
      idx.push_back(rem.at(i));
    }
  }
  rem.clear();
  rem = idx;
}

bool clean_fit(const string& w) {
  int r, c;
  Entry *e;
  size_t size = placed.size();
  for (int k=0; k < size; ++k) {
    e = &placed.at(k);
    for (int i=0; i < w.size(); ++i) {
      for (int j=0; j < e->word.size(); ++j) {
        if (w[i] == e->word[j]) {
          r = e->row;
          c = e->col;
          if (e->across) {
            r -= i;
            c += j;
          } else {
            c -= i;
            r += j;
          }
          if (r >= 0 && r < SIZE && c >= 0 && c < SIZE) {
            if (fit_word(w, r, c, !e->across)) {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

bool force_fit(const string& w) {
  char c;
  for (int i=0; i < SIZE; ++i) {
    for (int j=0; j < SIZE; ++j) {
      c = grid[i][j];
      if (c == EMPTY) {
        if (fit_word(w,i,j,true) || fit_word(w,i,j,false)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool fit_word(const string w, int x, int y, bool across) {
  int z = w.size() + (across? y : x);
  int i = 0;
  char c;
  if (x >= SIZE || x < 0 || y >= SIZE || y < 0 || z > SIZE) return false;
  if (across) {
    if (y > 0 && grid[x][y-1] != EMPTY || z < SIZE && grid[x][z] != EMPTY) return false;
    for (i=y; i<z; ++i) {
      c = grid[x][i];
      if (c == w[i-y]) continue;
      if ((c != EMPTY && c != w[i-y]) ||
          (x > 0 && grid[x-1][i] != EMPTY) ||
          (x < SIZE-1 && grid[x+1][i] != EMPTY))
        return false;
    }
    for (i=y; i<z; ++i)
      grid[x][i] = w[i-y];
  } else {
    if (x > 0 && grid[x-1][y] != EMPTY || z < SIZE && grid[z][y] != EMPTY) return false;
    for (i=x; i<z; ++i) {
      c = grid[i][y];
      if (c == w[i-x]) continue;
      if ((c != EMPTY && c != w[i-x]) ||
          (y != 0 && grid[i][y-1] != EMPTY) ||
          (y != SIZE -1 && grid[i][y+1] != EMPTY))
        return false;
    }
    for (i=x; i<z; ++i)
      grid[i][y] = w[i-x];
  }
  Entry e;
  e.word = w;
  e.across = across;
  e.row = x;
  e.col = y;
  placed.push_back(e);
  return true;
}

int main(int argc, char** argv) {
  
  /*if (argc == 1) {
    printf("Usage: grid [-s size] <input> <output>\n\nOptions:\n");
  }*/
  
  init(); //setup
  fill_grid(); //populate grid
  
  print_grid();
  //print_stats();

  fstream fs;
  fs.open("output.txt", fstream::out);
  fs<<ss.str();
  cout<<ss.str();
  fs.close();
  
  return 0;
}
