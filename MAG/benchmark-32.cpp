#include <iostream>
#include <fstream>
#include <string>
#include "Snap.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "stdafx.h"

using namespace std;
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}


int main(int argc,char* argv[]) {
  cerr<<"Started running code"<<currentDateTime()<<"\n";
  char src_file[] = "/dfs/scratch0/dataset/MicrosoftAcademicGraph/20160205/PaperAuthorAffiliations.txt";
  TTableContext Context;
  // Create schema.
  Schema PAA;
  PAA.Add(TPair<TStr,TAttrType>("PaperID", atStr));
  PAA.Add(TPair<TStr,TAttrType>("AuthorID", atStr));
  PAA.Add(TPair<TStr,TAttrType>("AfflID", atStr));
  PAA.Add(TPair<TStr,TAttrType>("AfflName", atStr));
  PAA.Add(TPair<TStr,TAttrType>("AfflNameNorm", atStr));
  PAA.Add(TPair<TStr,TAttrType>("AuthSeqNum", atInt));
  TIntV RelevantCols;
  RelevantCols.Add(0); RelevantCols.Add(1); RelevantCols.Add(2);
  RelevantCols.Add(3); RelevantCols.Add(4); RelevantCols.Add(5);
  cerr<<"loading table"<<currentDateTime()<<"\n";

  PTable P = TTable::LoadSS(PAA, src_file, &Context, RelevantCols);
  cerr<<"Table loaded"<<currentDateTime()<<"\n";

  {
  TFOut SOut("paperAuthAfflTable3.bin");
  P->Save(SOut);
  Context.Save(SOut);
  }

  cerr<<"Table saved"<<currentDateTime()<<"\n";
  // Test SaveSS by loading the saved table and testing values again.
  // To load table the saved table directly

  // PTable P;
  // {
  // TFIn SIn("/lfs/madmax6/0/mohanas/snap-dev/examples/cascadegen/paperAuthAfflTable3.bin");
  // TFIn SIn("paperAuthAfflTable3.bin");
  // cerr<<"Sin done";
  // P = TTable::Load(SIn, &Context);
  // Context.Load(SIn);
  // }
  // cerr<<"Table loaded "<<currentDateTime()<<endl;


  TVec<TPair<TStr, TAttrType> > S = P->GetSchema();
  TStrV NodeAttrV;
  PMMNet Graph = TMMNet::New();
  clock_t start = clock();
  TSnap::LoadModeNetToNet(Graph, "Author", P, "AuthorID", NodeAttrV);
  TSnap::LoadModeNetToNet(Graph, "Paper", P, "PaperID", NodeAttrV);
  TSnap::LoadModeNetToNet(Graph, "Affl", P, "AfflID", NodeAttrV);
  clock_t end = clock();
  double tim = (double) (end - start) / CLOCKS_PER_SEC;
  printf("Done loading all Mode, %f\n", tim);

  start = clock();
  TSnap::LoadCrossNetToNet(Graph, "Paper", "Author", "Paper-Author", P, "PaperID", "AuthorID", NodeAttrV);
  TSnap::LoadCrossNetToNet(Graph, "Paper", "Affl", "Paper-Affl", P, "PaperID", "AfflID", NodeAttrV);
  TSnap::LoadCrossNetToNet(Graph, "Author", "Affl", "Author-Affl", P, "AuthorID", "AfflID", NodeAttrV);
  end = clock();
  tim = (double) (end - start) / CLOCKS_PER_SEC;
  printf("Done loading all crossnets, %f\n", tim);

  cerr<<"Done convertion to MMNet"<<currentDateTime()<<"\n";
  TMMNet::TCrossNetI CI = Graph->GetCrossNetI(Graph->GetCrossId("Paper-Author"));
  cerr<<"paper auth edges "<< CI.GetCrossNet().GetEdges()<<endl;
  CI = Graph->GetCrossNetI(Graph->GetCrossId("Paper-Affl"));
  cerr<<"paper affl edges "<< CI.GetCrossNet().GetEdges()<<endl;
  CI = Graph->GetCrossNetI(Graph->GetCrossId("Author-Affl"));
  cerr<<"auth affl edges "<< CI.GetCrossNet().GetEdges()<<endl;


  printf("start convertion to TNEANet, %f\n", tim);
  start = clock();

  // For converting to PNEANet sequentially, use the following commented out lines
  //  TIntV crossnetids;
  //  crossnetids.Add(Graph->GetCrossId("Paper-Author"));
  //  crossnetids.Add(Graph->GetCrossId("Paper-Affl"));
  //  crossnetids.Add(Graph->GetCrossId("Author-Affl"));
  //
  //  TIntStrStrTrV nodeattrmapping;
  //  TIntStrStrTrV edgeattrmapping;
  //PNEANet pneanet = Graph->ToNetwork(crossnetids, nodeattrmapping, edgeattrmapping);

  TStrV crossnetnames;
  crossnetnames.Add("Paper-Author");
  crossnetnames.Add("Paper-Affl");
  crossnetnames.Add("Author-Affl");

  PNEANetMP pneanet = Graph->ToNetworkMP(crossnetnames);

  end = clock();
  tim = (double) (end - start) / CLOCKS_PER_SEC;
  printf("Done loading all crossnets, %f\n", tim);
  cerr<<"Done conversion to TNEANet"<<currentDateTime()<<"\n";

  cerr<<pneanet->GetNodes()<<" "<<pneanet->GetEdges()<<endl;
  
  cerr<<"triangle count starting"<<currentDateTime()<<"\n";
  start = clock();
  cerr<<TSnap::GetTriangleCnt(pneanet);
  end = clock();
  tim = (double) (end - start) / CLOCKS_PER_SEC;
  printf("triangle count, %f\n", tim);
  cerr<<"Triangle count done"<<currentDateTime()<<"\n";


  cerr<<"SCC calculation starting"<<currentDateTime()<<"\n";
  start = clock();
  TCnComV components;
  TSnap::GetSccs(pneanet, components);
  end = clock();
  tim = (double) (end - start) / CLOCKS_PER_SEC;
  printf("SCC calculation, %f\n", tim);
  cerr<<"SCC calculation starting"<<currentDateTime()<<"\n";





}
