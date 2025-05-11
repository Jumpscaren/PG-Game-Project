//#define _CRTDBG_MAP_ALLOC
#include "QREntryPoint.h"
//#include <stdlib.h>
//#include <crtdbg.h>
//#include <iostream>
#include "ScriptHelp/CharactersInterface.h"

QREntryPoint* entry_point;

int main()
{
	//_CrtMemState sOld;
	//_CrtMemState sNew;
	//_CrtMemState sDiff;
	//_CrtMemCheckpoint(&sOld); //take a snapshot

	entry_point = new QREntryPoint();

	entry_point->EntryPoint();

	CharactersInterface::RegisterInterface();

	entry_point->RunTime();

    entry_point->Cleanup();

    delete entry_point;

    //_CrtMemCheckpoint(&sNew); //take a snapshot 
    //if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
    //{
    //    //OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
    //    _CrtMemDumpStatistics(&sDiff);
    //    //OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
    //    _CrtMemDumpAllObjectsSince(&sOld);
    //    //OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
    //    _CrtDumpMemoryLeaks();
    //}

	return 0;
}