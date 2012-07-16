#!/usr/bin/env python
# 
# Copyright (C) University College London, 2007-2012, all rights reserved.
# 
# This file is part of HemeLB and is CONFIDENTIAL. You may not work 
# with, install, use, duplicate, modify, redistribute or share this
# file, or any part thereof, other than as allowed by any agreement
# specifically made by you with University College London.
# 

# encoding: utf-8
"""
results_collection.py

Created by James Hetherington on 2012-01-23.
Copyright (c) 2012 __MyCompanyName__. All rights reserved.
"""

import os
import result

class ResultsCollection(object):
    def __init__(self,source_path,config):
        self.source_path=source_path
        # Glob over the source path results collection
        results=os.listdir(os.path.expanduser(source_path))
        Result=result.result_model(config)
        self.results=[Result(os.path.join(source_path,res)) for res in results]
    def filter(self,selection,invert=False,latest=False):
        def filtration(result):
            ok=all([result.query(prop,value) for prop,value in selection.iteritems()])
            return ok if not invert else not ok
        results=filter(filtration,self.results)
        if latest:
            results=sorted(results,key=lambda result: result.report_created_timestamp)[-1*latest:]