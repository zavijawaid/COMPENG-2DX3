#!/usr/bin/env python
# coding: utf-8

# In[2]:


'''
  COMPENG 2DX3 Final Project
  Zavi Jawaid - jawaidz - 400368132
'''

import math
import serial

scan_sets = 3
scans = 32
angle = 2 * math.pi / scans

def main():
    
    #open up port 
    ser = serial.Serial('/dev/cu.usbmodemME4010231', 115200, timeout = 10)
    
    print("Opening: " + ser.name)
 
    while (True):
        try:
            line = ser.readline().decode("utf-8").rstrip() #read "start" message from micro
            print(line)
            
            if (line == "start"):
                print("collecting data...")
                
                for i in range(scan_sets):
                    file = open("data.txt", "a") #open up data.txt 
                    print(f'set-{i + 1}')

                    for j in range(scans): #32 times for 32 measurements
                        line = ser.readline().decode("utf-8").rstrip() #read distance measurement
                        print(line)
                        y = (int(line) / scan_sets) * math.sin(angle * j); #convert to y-val using sine
                        z = (int(line) / scan_sets) * math.cos(angle * j); #convert to x-val using cosine
                        file.write(f"{i * 50} {y} {z}\n") #write vals to the file
                    file.close() 
                break
        
        except KeyboardInterrupt: #stop if button gets pressed again
            exit()
            
            
if __name__ == "__main__":
    
    main()


# In[ ]:




