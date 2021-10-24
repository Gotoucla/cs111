#!/usr/bin/python

import csv, sys

super_block = {}
inodes = []
free_block = []
free_inode = []
direct = []
indirect = []

alloc_inode = []
unalloc_inode = []

block_ref = {}

level_pre = ["", "INDIRECT ", "DOUBLE INDIRECT ", "TRIPLE INDIRECT "]

class Super:
    def __init__(self, line):
        self.blocks_count = int(line[1])
        self.inodes_count = int(line[2])
        self.block_size = int(line[3])
        self.inode_size = int(line[4])
        self.blocks_per_group = int(line[5])
        self.inodes_per_group = int(line[6])
        self.first_non_reserved_inode = int(line[7])
        self.inode_table_num = self.inodes_count * self.inode_size / self.block_size + 1
                
        
class Inode:
    def __init__(self, line):
        self.inode_num = int(line[1])
        self.file_type = line[2]
        self.mode = int(line[3])
        self.owner = int(line[4])
        self.group = int(line[5])
        self.link_count = int(line[6])
        self.ctime = line[7]
        self.mtime = line[8]
        self.atime = line[9]
        self.file_size= int(line[10])
        self.num_block_used = int(line[11])
        self.direct = [int(val) for val in line[12:24]]
        self.single_indirect = int(line[24])
        self.double_indirect = int(line[25])
        self.triple_indirect = int(line[26])

class Direct:
    def __init__(self, line):
        self.parent_inode = int(line[1])
        self.offset = int(line[2])
        self.inode = int(line[3])
        self.entry_len = int(line[4])
        self.name_len = int(line[5])
        self.name = line[6]

class Indirect:
    def __init__(self, line):
        self.inode = int(line[1])
        self.level = int(line[2])
        self.offset = int(line[3])
        self.block_num = int(line[4])
        self.reference_block_num= int(line[5])
               
        
def file_content(file):
    global super_block,inodes,direct,indirect

    with file as fp:
        reader = csv.reader(fp)
        for line in reader:
            key = line[0]

            if key == 'SUPERBLOCK':
                super_block = Super(line)
            elif key == 'GROUP':
                pass
            elif key == 'BFREE':
                free_block.append(int(line[1]))
            elif key == 'IFREE':
                free_inode.append(int(line[1]))
            elif key == 'INODE':
                inodes.append(Inode(line))
            elif key == 'DIRENT':
                direct.append(Direct(line))
            elif key == 'INDIRECT':
                indirect.append(Indirect(line))

            else :
                sys.stderr.write("Invalid key words {} in csv file\n".format(key))
                exit(1)


def check_block(block_num,offset,inode_num,level):
    global block_ref

    if block_num != 0:
        
        if block_num < 0 or block_num > super_block.blocks_count:
            print("INVALID {}BLOCK {} IN INODE {} AT OFFSET {}".format(level_pre[level],block_num, inode_num, offset))

        elif block_num < super_block.inode_table_num + 4:
            print("RESERVED {}BLOCK {} IN INODE {} AT OFFSET {}".format(level_pre[level],block_num, inode_num, offset))
 
        elif block_num in block_ref:
            block_ref[block_num].append((inode_num, offset, level))
        #print("DUPLICATE {} BLOCK {} IN INODE {} AT OFFSET {}".format(level_pre[level], block_num, inode_num, offset))

       
        else:
            block_ref[block_num] = [(inode_num,offset,level)]
        
            
        

def inode_audits():

    global alloc_inode, unalloc_inode

    unalloc_inode = free_inode
    
    
    for inode in inodes:
        alloc_inode.append(inode)
        
        if inode.mode == 0 and inode.inode_num not in free_inode:
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(inode.inode_num))
            unalloc_inode.append(inode.inode_num)
            alloc_inode.remove(inode)
            
        elif inode.mode != 0 and inode.inode_num in free_inode:
            print("ALLOCATED INODE {} ON FREELIST".format(inode.inode_num))
            unalloc_inode.remove(inode.inode_num)
           
        
    for inode_num in range(super_block.first_non_reserved_inode,super_block.inodes_count):
        used = False
        match = list(filter(lambda x: x.inode_num == inode_num, inodes))
        if len(match) > 0:
            used = True
        
        if inode_num not in free_inode and not used:
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(inode_num))
            unalloc_inode.append(inode_num)
            
            
def direct_audits():
    inode_link = {}
    inode_parent = {2:2}
    
    for dir in direct:
        if dir.inode < 1 or dir.inode > super_block.inodes_count:
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(dir.parent_inode, dir.name, dir.inode))
        elif dir.inode in unalloc_inode:
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(dir.parent_inode, dir.name, dir.inode))

            # need else
            # count how many times each inode appear

        else :
            if dir.name != "'.'" and dir.name != "'..'":
                inode_parent[dir.inode] = dir.parent_inode
                
            inode_link[dir.inode] = inode_link.get(dir.inode,0) + 1

        
    # check all inodes' link count
    for inode in alloc_inode:
        if inode.inode_num in inode_link:
            if inode.link_count != inode_link[inode.inode_num]:
                print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode.inode_num, inode_link[inode.inode_num], inode.link_count))

        else:
            if inode.link_count != 0:
                print("INODE {} HAS 0 LINKS BUT LINKCOUNT IS {}".format(inode.inode_num, inode.link_count))


    # check "." and ".." 
    for dir in direct:
        if dir.name == "'.'":
            if dir.inode != dir.parent_inode:
                print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(dir.parent_inode, dir.inode, dir.parent_inode))
        elif dir.name == "'..'":
            if dir.inode != inode_parent[dir.inode]:
                print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(dir.parent_inode, dir.inode, inode_parent[dir.inode]))
  
def block_audits():

    for inode in inodes:

        if inode.file_type == 's' and inode.file_size <= 60:
            continue

        for offset, block_num in enumerate(inode.direct):
            check_block(block_num,offset,inode.inode_num,0)
            
        check_block(inode.single_indirect,12,inode.inode_num,1)
        check_block(inode.double_indirect,12+256,inode.inode_num,2)
        check_block(inode.triple_indirect,12+256+256*256,inode.inode_num,3)


    for indir in indirect:
        check_block(indir.reference_block_num,indir.offset,indir.inode, indir.level)

    for block in range(super_block.inode_table_num + 4,super_block.blocks_count):
        if block not in block_ref and block not in free_block:
            print("UNREFERENCED BLOCK {}".format(block))
        elif block in block_ref and block in free_block:
            print("ALLOCATED BLOCK {} ON FREELIST".format(block))
        elif block in block_ref and len(block_ref[block]) > 1:
            for inode_num, offset, level in block_ref[block]:
                print("DUPLICATE {}BLOCK {} IN INODE {} AT OFFSET {}".format(level_pre[level], block, inode_num, offset))
        

if __name__ == '__main__':
    #print("begin the csv file")
    if len(sys.argv) != 2:
        sys.stderr.write("Invalid arguments\n")
        sys.stderr.write("Usage: ./lab3b <file_name>")

        
    file = open(sys.argv[1],'r')
    if not file:
        sys.stderr.write("The file can't open\n")
        exit(1)
            
    file_content(file)

    inode_audits()
    direct_audits()
    block_audits()

    #print("finish the csv file")
           
    exit(0)
        
        
