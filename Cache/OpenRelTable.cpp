#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free=true;
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

  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID
  tableMetaInfo[0].free=false;
  tableMetaInfo[1].free=false;
  strcpy(tableMetaInfo[0].relName,RELCAT_RELNAME);
  strcpy(tableMetaInfo[1].relName,ATTRCAT_RELNAME);
}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
  for(int i=0;i<MAX_OPEN;i++)
  {
    if(strcmp(relName,tableMetaInfo[i].relName)==0)
    return i;
  }
  return E_RELNOTOPEN;
  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for(int i=2;i<MAX_OPEN;i++)
  {
    if(tableMetaInfo[i].free)
    return i;
  }
  return E_CACHEFULL;
  // if found return the relation id, else return E_CACHEFULL.
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int relId=OpenRelTable::getRelId(relName);
  if(relId !=E_RELNOTOPEN/* the relation `relName` already has an entry in the Open Relation Table */){
    // (checked using OpenRelTable::getRelId())
    return relId;
    // return that relation id;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
    relId=OpenRelTable::getFreeOpenRelTableEntry();

  if (relId==E_CACHEFULL/* free slot not available */){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute relval;
  strcpy(relval.sVal,relName);
  RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID,(char *)RELCAT_ATTR_RELNAME,relval,EQ);
  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  

  if (relcatRecId.block==-1 && relcatRecId.slot==-1/* relcatRecId == {-1, -1} */) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  RelCacheEntry * relcache=(RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  RelCatEntry relCatBuf;
  RecBuffer blkbuff(relcatRecId.block);
  Attribute record[RELCAT_NO_ATTRS];
  blkbuff.getRecord(record,relcatRecId.slot);
  RelCacheTable::recordToRelCatEntry(record,&relCatBuf);
  relcache->recId=relcatRecId;
  relcache->relCatEntry=relCatBuf;
  RelCacheTable::relCache[relId]=relcache;
  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* listHead=nullptr;
  AttrCacheEntry * prev=nullptr;
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);


  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  while(true)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;
      attrcatRecId=BlockAccess::linearSearch(ATTRCAT_RELID,(char *)ATTRCAT_ATTR_RELNAME,relval,EQ);
      if(attrcatRecId.slot==-1 && attrcatRecId.block==-1)
      break;
      AttrCacheEntry * entry=(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
      RecBuffer blkbuff(attrcatRecId.block);
      Attribute record[ATTRCAT_NO_ATTRS];
      blkbuff.getRecord(record,attrcatRecId.slot);
      AttrCatEntry attrcatbuf;
      AttrCacheTable::recordToAttrCatEntry(record,&attrcatbuf);
      entry->attrCatEntry=attrcatbuf;
      entry->recId=attrcatRecId;
      entry->next=nullptr;
      if(!listHead)
      listHead=entry;
      if(prev)
      {
        prev->next=entry;
      }
      prev=entry;
      


      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId]=listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/
  tableMetaInfo[relId].free=false;
  strcpy(tableMetaInfo[relId].relName,relName);
  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId==0 || relId==1/* rel-id corresponds to relation catalog or attribute catalog*/) {
    return E_NOTPERMITTED;
  }

  if (relId<0 || relId>=MAX_OPEN/* 0 <= relId < MAX_OPEN */) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free/* rel-id corresponds to a free slot*/) {
    return E_RELNOTOPEN;
  }
  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId]=nullptr;

  AttrCacheEntry * entry=AttrCacheTable::attrCache[relId];
  while(entry)
  {
    AttrCacheEntry * next=entry->next;
    free(entry);
    entry=next;
  }
  AttrCacheTable::attrCache[relId]=nullptr;
  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function

  tableMetaInfo[relId].free=true;
  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr

  return SUCCESS;
}


OpenRelTable::~OpenRelTable() {

  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  // free all the memory that you allocated in the constructor
  for(int i=2;i<MAX_OPEN;i++)
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