/*-----------------------------------------------------------------------------
 * File:    exG_main.cc
 * Author:  Daniel Luchaup
 * Date:    December 2013
 * Copyright 2013 Daniel Luchaup luchaup@cs.wisc.edu
 *
 * This file contains unpublished confidential proprietary work of
 * Daniel Luchaup, Department of Computer Sciences, University of
 * Wisconsin--Madison.  No use of any sort, including execution, modification,
 * copying, storage, distribution, or reverse engineering is permitted without
 * the express written consent (for each kind of use) of Daniel Luchaup.
 *
 *---------------------------------------------------------------------------*/
#include <iostream>
#include "gram.h"
#include "MyTime.h"
#include <cstdarg>
#include "exG_defs.h"
#include "gram_dbg.h"
#include "zoptions.h"
#include "dbg.h"
#include "MemoryInfo.h"


void test_unrank_rank(unsigned int max_len, unsigned int count);

void test_gram() {
  MyTime tm_test_gram("test_gram");
  unsigned int forced_len = zop.dbgNumOption("forced_len", 1000);
  /*** Initialize the global representation for the above grammar ***/
  {
    MyTime tm_init_G("init_G("+toString(forced_len)+")");
    MemoryInfo mi;
    mi.read();  mi.dump("memory-before-init_G"); std::cout << std::endl;
    init_G(forced_len);
    mi.read();  mi.dump("memory-after-init_G"); std::cout << std::endl;
    std::cout << "\n-------------- done init --------------\n";
  }
  std::cout << "forced_len=" << forced_len << std::endl;
  std::cout << "NUM_MATCHES=" << G.S()->num_matches(forced_len) << std::endl;
  test_unrank_rank(forced_len, 100);
}

///////////////////////////////////////////////////////////////////////////////
// -zforced_len:20
void get_options(int argc, char *argv[])
{
  int c;
  extern char *optarg;
  
  while ((c = getopt(argc, argv, "z:")) != -1) {
    switch (c)
      {
      case 'z':
	zop.dbgOptionSet.insert(std::string(optarg));
	break;
      default:
	zop.usage_and_die("unknown option");
	break;
      }
  }
  
  return;
}
void show_options(int argc, char *argv[]) {
  // Debug: dump the arguments
  std::cout << "Command line: ";
  for (int i = 0; i < argc; ++i) {
    std::cout << argv[i] << " ";
  }
  std::cout << std::endl;
#ifndef SVN_VERSION
#define SVN_VERSION "UNKNOWN"
#endif

#ifndef SVN_CHANGED
#define SVN_CHANGED "UNCHANGED"
#endif

  DMPNL(SVN_VERSION);
  DMPNL(SVN_CHANGED);
  
  zop.dumpOptions();
}

#define R_VERBOSE 1
void test_unrank_rank(unsigned int max_len, unsigned int count) {
  const Nonterminal *S = G.S();
  while (S->num_matches(max_len) == 0 && max_len > 0)
    --max_len;
  MPInt n = S->num_matches(max_len);
  if (n == 0)
    return;
  MPInt max_rank = n - 1;
  if (max_rank < count)
    count = max_rank.get_ui();
  if (count == 0)
    count = 1;
  assert(count > 0);
  MPInt step = max_rank/count;
  n = max_rank;
  bool allow_ambiguous = true;
  bool show_ambiguous = true;
  unsigned int ambiguous = 0;

  MyTime tm_test_gram("test_unrank_rank"+toString(max_len));
  MyTime tm_rank("rank::test_unrank_rank("+toString(max_len)+","+toString(count)+")", false);
  MyTime tm_unrank("unrank::test_unrank_rank("+toString(max_len)+","+toString(count)+")", false);
  MyTime tm_flatten("flatten::test_unrank_rank("+toString(max_len)+","+toString(count)+")", false);
  MyTime tm_pt_from_mem("pt_from_mem::test_unrank_rank("+toString(max_len)+","+toString(count)+")", false);
  for (unsigned int i = 0; i < count; ++i, (n = n - step)) {
    MPInt copy_n = n;
    tm_unrank.resume();
    ParseTree *pt_unrank = S->unrank(copy_n, max_len);
    tm_unrank.stop();
    
    tm_flatten.resume();
    std::string str = pt_unrank->flatten();
    tm_flatten.stop();
    delete pt_unrank;
#if R_VERBOSE    
    std::cout << std::endl << "str=" << str << std::endl;
#endif
    
    tm_pt_from_mem.resume();
    GRule::GParseTree *pt = static_cast<GRule::GParseTree*>(pt_from_mem(str));
    tm_pt_from_mem.stop();
    //std::string flat = pt->flatten();    assert(flat.compare(str) == 0);
    
    tm_rank.resume();
    MPInt r = S->rank(pt);
    tm_rank.stop();
    delete pt;
    
    bool ambiguous = ( r != n );
    if (allow_ambiguous) {
      if ( ambiguous) {
	ambiguous++;
	MPInt copy_r = r;
	ParseTree *r_pt_unrank = S->unrank(copy_r, max_len);
	std::string r_str = r_pt_unrank->flatten();
	assert(r_str.compare(str) == 0);
	delete r_pt_unrank;
#if R_VERBOSE
	if (show_ambiguous)
	  std::cout << std::endl << " " << max_len << " ambiguous: "  << str << std::endl;
#endif
      }
    }
    else
      assert(!ambiguous);//this fails for ambiguous grammars. Ex /(11*)*/
#if R_VERBOSE
    bool dbg_internal = true;
    if (dbg_internal) {
      std::cout << "Iteration i= " << i << std::endl
		<< " unrank(copy_n)= " << str << std::endl
		<< " ambiguous= " << ambiguous << std::endl;
    }
#endif
  }
  if (ambiguous)
    std::cout << std::endl << " " << max_len
	      << " ambiguous# " << ambiguous << std::endl;
}

int main(int argc, char *argv[]) {
  MyTime tm("main");
  get_options(argc, argv);
  show_options(argc, argv);
  test_gram();
  MemoryInfo mi; mi.read();  mi.dump("memory-end-main"); std::cout << std::endl;
}
