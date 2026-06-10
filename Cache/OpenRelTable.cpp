#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;
  

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  RecBuffer attrCatblock(RELCAT_BLOCK);
  Attribute attrcatrecord[RELCAT_NO_ATTRS];
  attrCatblock.getRecord(attrcatrecord,RELCAT_SLOTNUM_FOR_ATTRCAT);

  RelCacheEntry entry;
  RelCacheTable::recordToRelCatEntry(attrcatrecord,&entry.relCatEntry);
  entry.recId.block=RELCAT_BLOCK;
  entry.recId.slot=RELCAT_SLOTNUM_FOR_ATTRCAT;

  RelCacheTable::relCache[ATTRCAT_RELID]=(struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = entry;
  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  AttrCacheEntry * head=nullptr;
  AttrCacheEntry * prev=nullptr;
  for(int i=0;i<RELCAT_NO_ATTRS;i++)
  {
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry * entry=(struct AttrCacheEntry *)malloc(sizeof(struct AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&entry->attrCatEntry);
    entry->recId.block=ATTRCAT_BLOCK;
    entry->recId.slot=i;
    entry->next=nullptr;
    if(head==nullptr)
    head=entry;
    if(prev!=nullptr)
    prev->next=entry;
    prev=entry;
  }
  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc

  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head /* head of the linked list */;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  head=nullptr;
  prev=nullptr;
  for(int i=6;i<12;i++)
  {
    Attribute record[ATTRCAT_NO_ATTRS];
    attrCatBlock.getRecord(record,i);
    AttrCacheEntry * entry=(struct AttrCacheEntry *)malloc(sizeof(struct AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(record,&entry->attrCatEntry);
    entry->recId.block=ATTRCAT_BLOCK;
    entry->recId.slot=i;
    entry->next=nullptr;
    if(!head)
    {
        head=entry;
    }
    if(prev)
    {
        prev->next=entry;
    }
    prev=entry;
  }

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  AttrCacheTable::attrCache[ATTRCAT_RELID]=head;
}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
  for(int i=0;i<MAX_OPEN;i++)
  {
    if(RelCacheTable::relCache[i])
    {
        free(RelCacheTable::relCache[i]);
        RelCacheTable::relCache[i]=nullptr;
    }
  }

  for(int i=0;i<MAX_OPEN;i++)
  {
    AttrCacheEntry * head=AttrCacheTable::attrCache[i];
    if(head)
    {
        AttrCacheEntry *curr=head;
        
        while(curr)
        {
            AttrCacheEntry * next=curr->next;
            free(curr);
            curr=next;
            
        }
        AttrCacheTable::attrCache[i]=nullptr;
    }
  }
}