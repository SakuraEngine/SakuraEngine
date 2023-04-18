import os
import re

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))

class Access(object):
    def __init__(self, readonly, atomic, randomAccess, phase):
        self.readonly = readonly
        self.atomic = atomic
        self.randomAccess = randomAccess
        self.phase = phase

# literal grammar:
#   [access]<order>?type'
class Query(object):
    def __init__(self, literal : str):
        self.all = []
        self.any = []
        self.none = []
        self.components = []
        self.sequence = []
        self.accesses = []
        self.literal = literal
        #parse literal
        pattern = re.compile(r'\s+')
        literal = re.sub(pattern, '', literal)
        parts = literal.split(",")
        for part in parts:
            #[access]
            endpos = part.find("]")
            access = part[1:endpos]
            part = part[endpos+1:]
            #<order>
            order = "seq"
            if part[0] == "<":
                endpos = part.find(">")
                order = part[1:endpos]
                part = part[endpos+1:]
            if part[0] == "!":
                cat = self.none
                part = part[1:]
            elif part[0] == "?":
                cat = None
                part = part[1:]
            elif part[0] == "|":
                cat = self.any
                part = part[1:]
            else:
                cat = self.all
            #type
            endpos = part.find("'")
            if(endpos == -1):
                type = part
                count = -1
            else:
                type = part[:endpos]
                part = part[endpos+1:]
                #count tailing "'"
                count = 1
                while len(part) > count and part[count] == "'":
                    count += 1
            #add to list
            if access == "out":
                count = 0
            if access != "has" and cat is not self.none:
                if order != "unseq":
                    self.sequence.append(type)
                self.components.append(type)
                acc = Access(access == "in", access == "atomic", order, count)
                self.accesses.append(acc)
            if cat is not None:
                cat.append(type)

class Generator(object):
    def __init__(self):
        pass

    def filter_record(self, record):
        if hasattr(record.attrs, "query"):
            return True

    def filter_records(self, records):
        return [record for record in records if self.filter_record(record)]
    
    def parse(self, record):
        return Query(record.attrs.query)
        
    def generate_forward(self, db, args):
        template = os.path.join(BASE, "query.hpp.mako")
        if self.filter_records(db.records):
            return db.render(template, db=db, generator = self, api=args.api+"_API")
        return ""
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "query.cpp.mako")
        if self.filter_records(db.records):
            return db.render(template, db=db, generator = self)
        return ""

