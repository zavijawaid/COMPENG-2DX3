#!/usr/bin/env python
# coding: utf-8

# In[1]:


'''
  COMPENG 2DX3 Final Project
  Zavi Jawaid - jawaidz - 400368132
'''

import numpy as np
import open3d as o3d

scan_sets = 8
scans = 32

def main():
    
    #read data from xyz file
    pcd = o3d.io.read_point_cloud("data.txt", format = "xyz")
        
    lines = []
        
    #the inner-set connections
    for set in range(scan_sets):
        offset = scans * set
        for x in range(scans):
            
            if (x == scans - 1):
                lines.append([[x + offset], [x + offset - scans + 1]])                 
            else:
                lines.append([[x + offset], [x + offset + 1]])
                    
    #the inter-set connections
    for set in range(scan_sets - 1):
        offset = scans * set
        
    for x in range(scans):
        lines.append([[x + offset], [x + offset + scans]])
        
    line_set = o3d.geometry.LineSet(
        points = o3d.utility.Vector3dVector(np.asarray(pcd.points)),
        lines = o3d.utility.Vector2iVector(lines))
    
    o3d.visualization.draw_geometries([line_set])

if __name__ == "__main__":
    main()

