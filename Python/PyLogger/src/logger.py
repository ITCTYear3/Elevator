'''
 logger.py
 Command logger via socket connection and monitor serial port for incomming data

Created on Jun 10, 2014

@author: jmorgan1
'''

import sys
import os
import time
import datetime
import random
import threading
import select
import socket
import serial
import string
import wx
from wx.lib.pubsub import pub

# Preconfigured connections
# The local hostname determines the local port and the remote host to connect to
# The remote host must also be known so that the remote port can be looked up
#     {local hostname} : ( {local port}, {remote hostname} )
known_hosts = {
    "A3146-JM" : (31000, "A3146-04"),
    "A3146-04" : (31001, "A3146-JM"),
    "Chris-PC" : (31000, "Chris-PC"),
    "localhost" : (30999, "localhost")
}

# Attempt to determine the local host name and address 
# Returns a tuple (local_hostname, local_host)
def getLocalHostInfo():
    try:
        local_hostname = socket.getfqdn()
        local_host = socket.gethostbyname(local_hostname) # Use local interface IP address
    except socket.gaierror as e:
        try:
            local_hostname = socket.gethostname()
            local_host = socket.gethostbyname(local_hostname) # Try unqualified name if fqdn fails
        except socket.gaierror as e:
            local_hostname = "localhost"    # Last resort
            local_host = "127.0.0.1"
    return (local_hostname, local_host)
    
# Attempt to determine a (possibly remote) host address 
# Returns a tuple (hostname, host), falling back to localhost on failure
def getHostAddr(hostname):
    try:
        host = socket.gethostbyname(hostname)
    except socket.gaierror as e:
        hostname = "localhost"
        host = "127.0.0.1"
    return (hostname, host)
    
# Get local interface details
(local_hostname, local_host) = getLocalHostInfo();    
    
# Fall back to localhost if the local host name is not recognized
if local_hostname not in known_hosts:
    print "Local hostname not found in known_hosts. Falling back to localhost"
    print "Please create a connection profile for hostname: {}".format(local_hostname)
    local_hostname = "localhost"    
    
# Look up connection info for the chosen local hostname
(local_port, remote_hostname) = known_hosts[local_hostname]

# Fall back to localhost if the remote host name is not recognized
if remote_hostname not in known_hosts:
    print "Remote hostname not found in known_hosts. Falling back to localhost"
    print "Please create a connection profile for hostname: {}".format(remote_hostname)
    remote_hostname = "localhost"    

remote_port = known_hosts[remote_hostname][0]
(remote_hostname, remote_host) = getHostAddr(remote_hostname)
    
#local_port = 31000#random.randint(1025,36000) # Choose a random unprivileged port
#remote_host = '142.156.193.157'
#remote_port = 31001

print "Using connection profile:"
print "  {}:{} [{}] -> {}:{} [{}]".format(local_host, local_port, local_hostname, remote_host, remote_port, remote_hostname)        

local_socket = (local_host, local_port)
remote_socket = (remote_host, remote_port)


# System command tree
# Used for decoding payload data
cmds = { 
    0: ("Location", {
        "#": { 
           1: ("Up",{}), 2: ("Down",{}), 3: ("Stationary",{}) 
        }
    }),
    1: ("CallButton", {
        "#": {
           1: ("Up",{}), 2: ("Down",{})
        }
    }),
    2: ("PanelButton", {
        1: ("Floor1",{}), 2: ("Floor2",{}), 3: ("Floor3",{}), 4: ("Floor4",{}), 0x10: ("DoorOpen",{}), 0x11: ("DoorClose",{}), 0xEE: ("EmergencySTOP",{})
    }),
    3: ("PrintChar", {
        "#": {}
    }),
    0xFF: ("ERROR", {
        0: ("AllClear",{}), 1: ("GenericERROR",{})
    }),
}
            
        


class SerialClient(threading.Thread):
    """Busy wait watching serial port for new incomming data and pass data to subscriber"""
    def __init__(self, port='COM1', baudrate=9600, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE):
        threading.Thread.__init__(self)
        
        self.ser = serial.Serial()
        self.ser.port     = port
        self.ser.baudrate = baudrate
        self.ser.bytesize = bytesize
        self.ser.parity   = parity
        self.ser.rtscts   = False
        self.ser.xonxoff  = False
        self.ser.timeout  = 1    # Required so that the reader thread does not block indefinitely
        
        try:
            self.ser.open()
            print "Opened serial port {}".format(self.ser.name)
        except serial.SerialException as e:
            print "Could not open serial port {}: {}\n".format(self.ser.name, e)
            sys.exit(1)
        
        self.setDaemon(True)
        self.start()
    
    def run(self):
        self.reader_sm()
        
    def reader_sm(self):
        state = 'idh'
        frame = []
        while True:
            #try:                
            data = self.ser.read(1)              # read one byte, blocking
            if data == "":
                continue 
            data = ord(data)            
                
            # First byte of a new frame or string
            if state == 'idh':
                # Strings never start with null characters so this must be a frame
                if data == 0:
                    frame = [data]
                    state = 'idl'
                elif chr(data) in string.printable:
                #else:
                    str = chr(data)
                    state = 'str'
            # Found a string, read until a null character
            elif state == 'str':
                if data == 0:
                    state = 'idh'
                    print "String: \"{}\"".format(str)
                    msg = "String\n------\n{}\n\n".format(str)
                    wx.CallAfter(pub.sendMessage, 'update', data=msg)
                elif chr(data) in string.printable:
                #elif True:
                    str = str + chr(data)
                else:
                    state = 'idh'
                    print "Unprintable string; realigning..."
            # Found a frame, read the remaining bytes
            elif state == 'idl':
                frame = frame + [data]
                state = 'priority'
            elif state == 'priority':
                frame = frame + [data]
                state = 'length'
            elif state == 'length':       
                frame = frame + [data]
                payload = []
                length = data
                if length == 0:
                    state = 'frame'
                else:
                    state = 'payload'
            elif state == 'payload':   
                frame = frame + [data]
                payload = payload + [data]
                length = length - 1 
                if length == 0:
                    state = 'frame'
                    
            if state == 'frame':
                state = 'idh'
                # Parse payload data 
                buf = payload
                cmd = cmds
                payload_decode = ""
                try:
                    while buf != []:
                        if "#" in cmd:
                            payload_decode = "{}{} ".format(payload_decode, buf[0])
                            cmd = cmd["#"]
                        else:
                            payload_decode = payload_decode + cmd[buf[0]][0] + " "
                            cmd = cmd[buf[0]][1]
                        buf = buf[1:]
                except KeyError as ke:
                    print ke
                    print "WARNING: Unable to decode payload data"
                    payload_decode = "[INVALID KEY]"
                except IndexError as ie:
                    print ie
                    print "WARNING: Unable to decode payload data"
                    payload_decode = "[INVALID INDEX]"
                
                print "Frame: {} \"{}\"".format(frame, payload_decode)

                # Print frame to window
                id = frame[0] * (2^8) + frame[1]
                priority = frame[2]
                length = frame[3]
                #if ( length > 0 and payload[0] != 0 ):  # Filter location spam
                if True:
                    msg = "Frame\n------\nID: {}\nPriority: {}\nLength: {}\nPayload: {} \"{}\"\n\n".format(id, priority, length, payload, payload_decode)
                    wx.CallAfter(pub.sendMessage, 'update', data=msg)
        
        # Close serial connection after breaking out of the running loop
        try:
            self.ser.close()
        except serial.SerialException as e:
            print "Serial close error: {}".format(e)
            sys.exit(1)
    
    def reader(self):
        """loop forever and watch for messages on serial"""
        while True:
            try:
                data = self.ser.read(1)              # read one, blocking
                n = self.ser.inWaiting()             # look if there is more
                if n:
                    print "Lenth: {}".format(n+1)
                    data = data + self.ser.read(n)   # and get as much as possible
                if data:
                    """NOTE: data size is a max of 8 bytes for each read loop"""
                    print "Data: {}".format(data)
                    num_data = [ ord(ch) for ch in data ]
                    print "Data: {}".format(num_data)
                    wx.CallAfter(pub.sendMessage, 'update', data=data)
            except self.ser.SerialException as e:
                print "Serial read error: {}".format(e)
                break
        
        # Close serial connection after breaking out of the running loop
        try:
            self.ser.close()
        except serial.SerialException as e:
            print "Serial close error: {}".format(e)
            sys.exit(1)


class SocketClient(threading.Thread):
    """Listen on local socket and pass received data to subscriber"""
    def __init__(self):
        threading.Thread.__init__(self)
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind(local_socket)
        self.socket.listen(5)
        print "Listening on {}:{} [{}]".format(local_host, local_port, local_hostname)
        self.setDaemon(True)
        self.start()
    
    def run(self):
        while True:
            try:
                (client, addr) = self.socket.accept()
                ready = select.select([client,], [], [], 2)
                if ready[0]:
                    data = client.recv(4096)
                    data = str(addr) + " " + data   # Add on socket address
                    data = self.AddTimestamp(data)
                    print data
                    data += "\n"
                    # Tell the window panel to update the text area
                    wx.CallAfter(pub.sendMessage, 'update', data=data)
            except socket.error as e:
                print "Socket error: {}".format(e)
                break
            
        # Shutdown the socket after breaking out of the running loop
        try:
            self.socket.shutdown(socket.SHUT_RDWR)
        except:
            pass
        
        self.socket.close()
    
    def AddTimestamp(self, string):
        """Prepend a timestamp to a string"""
        ts = time.time()
        timestamp = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        return timestamp + " " + string


class MainPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        
        pub.subscribe(self.UpdateDisplay, 'update')
        
        self.InitUI()
    
    def InitUI(self):
        self.mainSizer = wx.BoxSizer(wx.VERTICAL)
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        textTitle = wx.StaticText(self, label="Logs")
        hbox1.Add(textTitle)
        self.mainSizer.Add(hbox1, flag=wx.LEFT|wx.TOP, border=5)
        
        hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.textDisplay = wx.TextCtrl(self, value="", style=wx.TE_MULTILINE|wx.TE_READONLY)
        hbox2.Add(self.textDisplay, proportion=1, flag=wx.EXPAND)
        self.mainSizer.Add(hbox2, proportion=1, flag=wx.LEFT|wx.RIGHT|wx.EXPAND, border=5)
        
        hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        btn1 = wx.Button(self, label="Clear log screen")
        btn1.Bind(wx.EVT_BUTTON, self.OnClear)
        hbox3.Add(btn1)
        self.mainSizer.Add(hbox3, flag=wx.ALIGN_CENTER)
        
        hbox4 = wx.BoxSizer(wx.HORIZONTAL)
        self.textInput = wx.TextCtrl(self, value="Test message", style=wx.TE_PROCESS_ENTER)
        self.textInput.Bind(wx.EVT_KEY_DOWN, self.OnEnter)
        hbox4.Add(self.textInput, proportion=1)
        btn2 = wx.Button(self, label="Send Message")
        btn2.Bind(wx.EVT_BUTTON, self.OnSendMsg)
        hbox4.Add(btn2)
        self.mainSizer.Add(hbox4, flag=wx.ALL|wx.EXPAND, border=5)
        
        hbox5 = wx.BoxSizer(wx.VERTICAL)
        
        btn3 = wx.Button(self, label="Log to file")
        btn3.Bind(wx.EVT_BUTTON, self.OnLog2File)
        hbox5.Add(btn3, flag=wx.LEFT|wx.RIGHT, border=5)
        self.pathText = wx.StaticText(self, label="No file selected")
        hbox5.Add(self.pathText, flag=wx.LEFT|wx.RIGHT, border=5)
        self.mainSizer.Add(hbox5)
        
        self.SetSizer(self.mainSizer)
    
    def OnEnter(self, event):
        """Handle when the return key is pressed within textInput field"""
        key = event.GetKeyCode()
        if key == wx.WXK_RETURN:
            self.OnSendMsg(event)   # Pass off to OnSendMsg function
        event.Skip()
    
    def OnSendMsg(self, event):
        """Called when the send button is pressed"""
        message = self.textInput.Value  # Get string from input field
        if message: # Only send if non-empty string
            try:
                print "Sending to {}:{}".format(remote_host, remote_port)
                client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client.connect(remote_socket)
                client.send(message)
                client.shutdown(socket.SHUT_RDWR)
                client.close()
                self.textInput.Clear()  # Clear text input field after sending successfully
            except socket.error as e:
                print "Socket error: {}".format(e)
    
    def OnClear(self, event):
        """Called when the clear logs button is pressed"""
        self.textDisplay.Clear()
    
    def UpdateDisplay(self, data):
        """Called when the window panel receives an event from pub.sendMessage with data"""
        self.textDisplay.AppendText( str(data) )
    
    def OnLog2File(self, event):
        """Called when the Log to File button is pressed"""
        dlg = wx.FileDialog(self,
                            message="Save as...",
                            defaultDir=os.getcwd(),
                            wildcard="Log files (*.log)|*.log",
                            style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
        result = dlg.ShowModal()
        if result == wx.ID_OK:
            path = dlg.GetPath()
        dlg.Destroy()
        self.pathText.SetLabel(str(path))   # Update file path text label


class MainWindow(wx.Frame):
    def __init__(self, parent, title, size=(500, 400)):
        wx.Frame.__init__(self, parent, title=title, size=size)
        
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        
        self.panel = MainPanel(self)
        self.Center()
        self.Show()
        
        # Start thread to listen for incomming socket connections
        self.ipc = SocketClient()
    
    def OnClose(self, event):
        """Called when the window close (X) button is pressed"""
        dlg = wx.MessageDialog(self,
                               "Are you sure you want to close the application?",
                               "Confirmation",
                               wx.YES_NO | wx.NO_DEFAULT | wx.ICON_QUESTION)
        result = dlg.ShowModal()
        dlg.Destroy()
        if result == wx.ID_YES:
            self.Destroy()


if __name__ == "__main__":
    app = wx.App(False)
    
    #local_socket = ('localhost', 8081); remote_socket = ('localhost', 8082)
    frame = MainWindow(None, title="Serial and Socket Logger 1")
    
    #local_socket = ('localhost', 8082); remote_socket = ('localhost', 8081)
    #frame2 = MainWindow(None, title="Serial and Socket Logger 2")
    
    # Start serial thread which will monitor serial port
    # and send data to the text display on the GUI window
    serial = SerialClient(port='COM11', baudrate=9600)
    
    app.MainLoop()
