#include "QREntryPoint.h"

QREntryPoint* entry_point;

int main()
{
	entry_point = new QREntryPoint();

	entry_point->EntryPoint();

	entry_point->RunTime();

	return 0;
}