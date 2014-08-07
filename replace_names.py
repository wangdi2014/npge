#!/usr/bin/python

import sys
import re
import argparse

def replace_names(args):
    name2name = {}
    for line in args.table:
        line = line.strip()
        if line:
            try:
                fasta_id, genome, chromosone, circular = line.split()[:4]
            except:
                print 'Warning: bad line in table: ' + line
            name2name[fasta_id] = genome + '&' + chromosone + '&' + circular
    for line in args.fasta:
        if line.startswith('>'):
            name, descr = line[1:].split(" ", 1)
            for k, v in name2name.items():
                if k in name:
                    name = v
                    descr = 'ac=' + k + ' ' + descr.strip()
                    break
            if name == line[1:].split(" ", 1)[0]:
                print 'Warning: name ' + name + ' was not replaced'
            line = '>' + name + ' ' + descr
        args.out.write(line)

def main():
    r = argparse.FileType('r')
    w = argparse.FileType('w')
    p = argparse.ArgumentParser(
        description='Replace fasta identifiers with genome and chromosone',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    p.add_argument('--fasta', help='Input fasta file',
            metavar='FILE', type=r, required=True)
    p.add_argument('--table',
            help='Table (fasta_id genome chromosone c[ircular]/l[inear])',
            metavar='FILE', type=r, required=True)
    p.add_argument('--out', help='Output new fasta file',
            metavar='FILE', type=w, required=True)
    args = p.parse_args()
    replace_names(args)

if __name__ == '__main__':
    try:
        main()
    except Exception, e:
        print e
        sys.exit(255)

