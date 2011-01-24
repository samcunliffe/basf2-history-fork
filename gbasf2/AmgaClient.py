#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Implementation of AmgaClient class used for communication with AMGA metadata catalog
# Author: Milosz Zdybal (milosz.zdybal@ifj.edu.pl)
# 2010-01
# Editor: Tom Fifield (fifieldt@unimelb.edu.au)
# 2010-11

import mdclient
import mdinterface
import sys
import os
from socket import gethostname

sys.path.append('../')


class AmgaClient(object):

    '''
    Module to handle connection with AMGA metadata-base
    '''

    user_dataset_attributes = {
        'id': 'int',
        'guid': 'varchar(32)',
        'lfn': 'varchar(1024)',
        'status': 'varchar(128)',
        'events': 'int',
        'experiment': 'int',
        'stream': 'smallint',
        'runL': 'smallint',
        'eventL': 'int',
        'runH': 'smallint',
        'eventH': 'int',
        'parentid': 'int',
        'versionid': 'smallint',
        'date': 'timestamp',
        'site': 'varchar(32)',
        'software': 'varchar(32)',
        'user': 'varchar(32)',
        'log': 'varchar(32)',
        }

    config = None
    client = None

###############################################################################

    def __init__(self):
        '''
        Constructor - needs path of yaml file with configuration
        '''

        try:
            # First, try to connect a proxy
            if os.environ.has_key('X509_USER_PROXY') and not gethostname() \
                == 'kek2-uidev.cc.kek.jp':
                print 'using X509 auth'
                # self.client = mdclient.MDClient('150.183.246.196', 8822, '')
                self.client = mdclient.MDClient('cgh10.collab.unimelb.edu.au',
                        8822, '')
                self.client.requireSSL(os.environ['X509_USER_PROXY'],
                                       os.environ['X509_USER_PROXY'])
            else:
                print 'using password auth'
              # self.client = mdclient.MDClient('amga.ijs.si', 8822, '', '')
                self.client = mdclient.MDClient('cgh10.collab.unimelb.edu.au',
                        8822, 'belle_user', 'belle')
        except Exception, ex:
              # self.client = mdclient.MDClient('cgh7.collab.unimelb.edu.au', 8822, '', '')
              # self.client = mdclient.MDClient('150.183.246.196', 8822,
              #         'belle_user', 'belle')
            print 'Count not connect to AMGA server:', ex

        self.client.execute('whoami')
        print self.client.fetchRow()

###############################################################################

    def getSubdirectories(self, dir, relative=False):
        '''
        Get subdirectories in metadata catalog.
        '''

        result = []
        self.client.execute('ls ' + dir)
        if relative or dir == '/':
            while not self.client.eot():
                result.append(self.client.fetchRow().split('/')[-1])
        else:
            while not self.client.eot():
                result.append(dir + '/' + self.client.fetchRow().split('/'
                              )[-1])
        return result

###############################################################################

    def getGUIDs(self, experiment, query):
        '''
        Old way of getting LFNs filfilling query.
        '''

        guids = []
        self.client.selectAttr([experiment], query)
        while not self.client.eot():
            guids.append('guid:' + self.client.fetchRow())
            # -  we're interesteed in LFNS now
            # guids.append(self.client.fetchRow())
        return guids

###############################################################################

    def directQuery(self, experiment, query):
        '''
        Query with ability to define parameters in SQL way.
        Returns list of LFNs.
        '''

        tmp = []
        results = []

        self.client.find(experiment, query)
        while not self.client.eot():
            tmp.append(experiment + '/' + self.client.fetchRow())

        for t in tmp:
            self.client.getattr(t, ['lfn', 'events'])
            self.client.fetchRow()
            results.append(self.client.fetchRow())

        return results

###############################################################################

    def directQueryWithAttributes(
        self,
        experiment,
        query,
        attributes,
        ):
        '''
        Query with ability to define parameters in SQL way.
        Returns dict of attribute values, indexed by path and attribute names

        Experiments come in as /VO/dataType/EXX/FC
        Query is a string
        attributes is an array of strings
        '''

        attrpos = {}
        lfnpos = 0
        results = {}

        print 'SELECT ' + ', '.join(attributes) + ' FROM ' + experiment \
            + ' WHERE ' + query.replace(' and ', ' AND ')

        self.client.execute('SELECT ' + ', '.join(attributes) + ' FROM '
                            + experiment + ' WHERE ' + query.replace(' and ',
                            ' AND '))

        # get the attribute positions
        for i in range(len(attributes)):
            row = self.client.fetchRow()
            attrpos[i] = row
            if row == 'lfn':
                lfnpos = i

        while not self.client.eot():
            tmp = {}
            row = ''
            tmplfn = None
          # grab the next row
            for i in range(len(attributes)):
                row = self.client.fetchRow()
                tmp[attrpos[i]] = row
                if i == lfnpos:
                    tmplfn = row

            results[tmplfn] = tmp

        # self.client.find(experiment, query)
        # while not self.client.eot():
        #    tmp.append(experiment + '/' + self.client.fetchRow())

        # for t in tmp:
        #    self.client.getattr(t, attributes)
        #  # ignore the first row
        #    self.client.fetchRow()
        #  # attributes are returned as a dict (index is path) of dicts
        #    results[t] = {}
        #    for attribute in attributes:
        #        results[t][attribute] = self.client.fetchRow()

        print results

        return results

###############################################################################

    def checkDirectory(self, path):
        '''
      Checks the directory exists, and creates if it is doesn't
      To be used before insertions
      '''

        try:
            self.client.cd(path)
        except mdinterface.CommandException, ex:
        # it doesn't exist - create
            try:
                self.checkDirectory('/'.join(path.split('/')[0:-1]))
                self.client.createDir(path)
            except mdinterface.CommandException, ex2:
                print 'Error:', ex2
                return False
        return True

###############################################################################

    def bulkInsert(self, basepath, entries):
        '''
      Loops through a list of entries, inserting as efficiently as possible
      entries[entry_name] = (['key1', 'key2', 'key3'],['value1', value2, value3])
      TODO: use direct SQL mode.
      '''

        if self.checkDirectory(basepath):
            for entry in entries.keys():
                try:
                    self.client.addEntry(basepath + '/' + entry,
                            entries[entry][0], entries[entry][1])
                except mdinterface.CommandException, ex:
                    print 'Error:', ex
                    return False
            return True
        else:
            return False

###############################################################################

    def getAttributes(self, basepath):
        '''
      returns the attributes in a given directory, or False on failure
      '''

        try:
            return self.client.listAttr(basepath)
        except mdinterface.CommandException, ex:
            print 'Error:', ex
            return False

###############################################################################

    def prepareUserDataset(self, basepath):
        '''
      Adds the set of stock attributes to the given basepath
      '''

        if self.checkDirectory(basepath):
            for key in self.user_dataset_attributes.keys():
                try:
                    self.client.addAttr(basepath, key,
                            self.user_dataset_attributes[key])
                except mdinterface.CommandException, ex:
                    print 'Error:', ex
                    return False
        else:
            return False


