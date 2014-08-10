'''
logger.py

Command logger via socket connection and monitor serial port for incomming data
For ElevatorControl Project, ESE semester 6

Created on Jun 10, 2014

@author: jmorgan, cbrown
'''

import sys
import os
import time
import datetime
import string
import threading
import select
import socket
import hashlib
import serial
import wx
from wx.lib.pubsub import pub

# Preconfigured connections
# The local hostname determines the local port and the remote host to connect to
# The remote host must also be known so that the remote port can be looked up
#     {local hostname} : ( {local port}, {remote hostname} )
known_hosts = {
    "A3146-08.conestogac.on.ca" : (31000, "A3146-04.conestogac.on.ca"),
    "A3146-04.conestogac.on.ca" : (31001, "A3146-08.conestogac.on.ca"),
    "Chris-PC" : (31000, "Chris-PC"),
    "localhost" : (30999, "localhost")
}


def getLocalHostInfo():
    """Attempt to determine the local host name and address 
       Returns a tuple (local_hostname, local_host)"""
    try:
        local_hostname = socket.getfqdn()
        local_host = socket.gethostbyname(local_hostname) # Use local interface IP address
    except socket.gaierror:
        try:
            local_hostname = socket.gethostname()
            local_host = socket.gethostbyname(local_hostname) # Try unqualified name if fqdn fails
        except socket.gaierror:
            local_hostname = "localhost"    # Last resort
            local_host = "127.0.0.1"
    return (local_hostname, local_host)


def getHostAddr(hostname):
    """Attempt to determine a (possibly remote) host address
       Returns a tuple (hostname, host), falling back to localhost on failure"""
    try:
        host = socket.gethostbyname(hostname)
    except socket.gaierror:
        hostname = "localhost"
        host = "127.0.0.1"
    return (hostname, host)


# Get local interface details
(local_hostname, local_host) = getLocalHostInfo();

# Fall back to localhost if the local host name is not recognized
if local_hostname not in known_hosts:
    print "Local hostname {} not found in known_hosts. Falling back to localhost".format(local_hostname)
    print "Please create a connection profile for {}".format(local_hostname)
    local_hostname = "localhost"

# Look up connection info for the chosen local hostname
(local_port, remote_hostname) = known_hosts[local_hostname]

# Fall back to localhost if the remote host name is not recognized
if remote_hostname not in known_hosts:
    print "Local hostname {} not found in known_hosts. Falling back to localhost".format(remote_hostname)
    print "Please create a connection profile for {}".format(remote_hostname)
    (local_hostname, local_host) = ("localhost", "127.0.0.1")
    (local_port, remote_hostname) = known_hosts[local_hostname]

remote_port = known_hosts[remote_hostname][0]
(remote_hostname, remote_host) = getHostAddr(remote_hostname)

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
        
        if port == 'COM1':
            print "Defaulting to port COM1"
        
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
    
    def SendFrame(self, id, priority=1, payload=[]):
        """Write frame data to serial port"""
        idh = int(id/256) # upper byte
        idl = id % 256    # lower byte
        length = len(payload)
        frame = [idh, idl, priority, length, payload]
        print "Sending frame: {}".format(frame)
        while len(frame) > 1:
            #print frame[0]    # Verbose debug
            self.ser.write(chr(frame[0]))
            frame = frame[1:]
        while len(payload) > 0:
            #print payload[0]  # Verbose debug
            self.ser.write(chr(payload[0]))
            payload = payload[1:]
        
    def reader_sm(self):
        """Poll serial port for incomming data and
           parse it to determine if it is a frame or a plain string"""
        state = 'idh'
        frame = []
        while True:
            data = self.ser.read(1) # read one byte, blocking
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
                    str += chr(data)
                else:
                    state = 'idh'
                    print "Unprintable string; realigning..."
            # Found a frame, read the remaining bytes
            elif state == 'idl':
                frame += [data]
                state = 'priority'
            elif state == 'priority':
                frame += [data]
                state = 'length'
            elif state == 'length':
                frame += [data]
                payload = []
                length = data
                if length == 0:
                    state = 'frame'
                else:
                    state = 'payload'
            elif state == 'payload':
                frame += [data]
                payload += [data]
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
                            payload_decode += cmd[buf[0]][0] + " "
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
                
                payload_decode = payload_decode.strip() # Strip out any leading or trailing whitespace
                print "Frame received: {} \"{}\"".format(frame, payload_decode)
                
                # Print frame to window
                id = frame[0] * (2^8) + frame[1]
                priority = frame[2]
                length = frame[3]
                #if ( length > 0 and payload[0] != 0 ):  # Filter location spam
                if True:
                    msg = "Frame\n------\nID: {}\nPriority: {}\nLength: {}\nPayload: {} \"{}\"\n\n".format(id, priority, length, payload, payload_decode)
                    wx.CallAfter(pub.sendMessage, 'update', data=msg)
                    if payload_decode.split()[0] == cmds[0][0]:
                        wx.CallAfter(pub.sendMessage, 'updateFloor', data=payload_decode.split()[1])
                        wx.CallAfter(pub.sendMessage, 'updateDirection', data=payload_decode.split()[2])   # Send update to node emu panel to show current floor
        
        # Close serial connection after breaking out of the running loop
        try:
            self.ser.close()
        except serial.SerialException as e:
            print "Serial close error: {}".format(e)
            sys.exit(1)
    
    def reader(self):
        """loop forever and watch for messages on serial - (Old method, no longer used)"""
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
            
        # Attempt to shutdown the socket gracefully
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


class NodeEmuPanel(wx.Panel):
    """CAN Node Emulation"""
    def __init__(self, parent, style):
        wx.Panel.__init__(self, parent=parent, style=style)
        
        pub.subscribe(self.UpdateFloor, 'updateFloor')
        pub.subscribe(self.UpdateDirection, 'updateDirection')
        
        self.InitUI()
    
    def InitUI(self):
        
        self.numFloors = 3
        
        
        #---- Current floor and direction text
        self.st_curfloor = wx.StaticText(self, label="Current floor: None", style=wx.BORDER_SIMPLE)
        curfloorSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(curfloorSizer, 5)
        curfloorSizer.Add(self.st_curfloor, 1)
        self.AddLinearSpacer(curfloorSizer, 5)
        
        self.st_curdir = wx.StaticText(self, label="Current direction: None", style=wx.BORDER_SIMPLE)
        curdirSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(curdirSizer, 5)
        curdirSizer.Add(self.st_curdir, 1)
        self.AddLinearSpacer(curdirSizer, 5)
        
        
        #---- Node sizers
        nodeSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(nodeSizer, 5)
        for floor in xrange(1, self.numFloors+1):
            nodeSizer.Add(self.AddNodeCtrl(floor))
            self.AddLinearSpacer(nodeSizer, 5)
        
        
        #---- Elevator car sizer
        carSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(carSizer, 5)
        carSizer.Add(self.AddCarCtrl())
        self.AddLinearSpacer(carSizer, 5)
        
        
        #---- Main vertical sizer
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(curfloorSizer, flag=wx.EXPAND)
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(curdirSizer, flag=wx.EXPAND)
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(nodeSizer, flag=wx.ALIGN_CENTER)
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(carSizer, flag=wx.ALIGN_CENTER)
        self.AddLinearSpacer(mainSizer, 5)
        
        self.SetSizer(mainSizer)    # Associate sizer to wx.Panel control
        self.Layout()
    
    def AddLinearSpacer(self, boxsizer, pixelSpacing):
        """A one-dimensional spacer along only the major axis for any BoxSizer"""
        orientation = boxsizer.GetOrientation()
        if orientation == wx.HORIZONTAL:
            boxsizer.Add( (pixelSpacing, 0) )
        elif orientation == wx.VERTICAL:
            boxsizer.Add( (0, pixelSpacing) )
    
    def AddNodeCtrl(self, floor):
        """Callbox interface"""
        st_NodeTitle = wx.StaticText(self, label="Callbox {}".format(floor))
        
        btn_Node1 = wx.ToggleButton(self, label="Up")
        #btn_Node1.Bind(wx.EVT_BUTTON, lambda event: serialObj.SendFrame(id=1, payload=[1, floor, 1]) ) # 1=up
        btn_Node1.Bind(wx.EVT_TOGGLEBUTTON,
                       lambda event: self.OnToggleClick(event, btn_Node1, id=1, payload=[1, floor, 1]) )
        
        btn_Node2 = wx.ToggleButton(self, label="Down")
        #btn_Node2.Bind(wx.EVT_BUTTON, lambda event: serialObj.SendFrame(id=1, payload=[1, floor, 2]) ) # 2=down
        btn_Node2.Bind(wx.EVT_TOGGLEBUTTON,
                       lambda event: self.OnToggleClick(event, btn_Node2, id=1, payload=[1, floor, 2]) )
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(st_NodeTitle, flag=wx.ALIGN_CENTER)
        sizer.Add(btn_Node1)
        self.AddLinearSpacer(sizer, 1)
        sizer.Add(btn_Node2)
        
        return sizer
    
    def AddCarCtrl(self):
        """Elevator car interface"""
        st_CarTitle = wx.StaticText(self, label="Elevator Car")
        
        btn_emerg = wx.ToggleButton(self, label="Emergency Stop")
        #btn_emerg.Bind(wx.EVT_TOGGLEBUTTON, lambda event: serialObj.SendFrame(id=1, payload=[2, 0xEE]) ) # 0xEE=emergency stop
        btn_emerg.Bind(wx.EVT_TOGGLEBUTTON,
                       lambda event: self.OnToggleClick(event, btn_emerg, id=1, payload=[2, 0xEE]) )
        
        hSizer = wx.BoxSizer(wx.HORIZONTAL)
        for floor in xrange(1, self.numFloors+1):
            btn_floor = wx.ToggleButton(self, label="Floor {}".format(floor))
            #btn_floor.Bind(wx.EVT_TOGGLEBUTTON, lambda event: serialObj.SendFrame(id=1, payload=[2, floor]) )
            btn_floor.Bind(wx.EVT_TOGGLEBUTTON,
                           lambda event, btn=btn_floor, f=floor: self.OnToggleClick(event, btn, id=1, payload=[2, f]) )
            hSizer.Add(btn_floor)
            self.AddLinearSpacer(hSizer, 5)
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(st_CarTitle, flag=wx.ALIGN_CENTER)
        self.AddLinearSpacer(sizer, 1)
        sizer.Add(hSizer)
        self.AddLinearSpacer(sizer, 1)
        sizer.Add(btn_emerg)
        
        return sizer
    
    def OnToggleClick(self, event, button, id, payload):
        """Change colour of toggle button when pressed and
           Send out message over serial"""
        if button.GetValue():
            button.SetBackgroundColour("Yellow")
            serialObj.SendFrame(id=id, payload=payload)
        else:
            button.SetBackgroundColour(wx.NullColour)
    
    def UpdateFloor(self, data):
        """Update current floor text"""
        self.st_curfloor.SetLabel("Current floor: {}".format(data))
    
    def UpdateDirection(self, data):
        """Update current direction text"""
        self.st_curdir.SetLabel("Current direction: {}".format(data))


class LoggingPanel(wx.Panel):
    def __init__(self, parent, style):
        wx.Panel.__init__(self, parent=parent, style=style)
        
        pub.subscribe(self.UpdateDisplay, 'update')
        
        self.InitUI()
    
    def InitUI(self):
        
        #---- Log display and associated buttons
        st_logTitle = wx.StaticText(self, label="Logs")
        
        self.tc_logDisplay = wx.TextCtrl(self, style=wx.TE_MULTILINE|wx.TE_READONLY)
        
        btn_clearLog = wx.Button(self, label="Clear log screen")
        btn_clearLog.Bind(wx.EVT_BUTTON, self.OnClear)
        
        logs_vSizer = wx.BoxSizer(wx.VERTICAL)
        logs_vSizer.Add(st_logTitle)
        logs_vSizer.Add(self.tc_logDisplay, proportion=1, flag=wx.EXPAND)
        logs_vSizer.Add(btn_clearLog, flag=wx.ALIGN_CENTER)
        
        logs_hSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(logs_hSizer, 5)
        logs_hSizer.Add(logs_vSizer, proportion=1, flag=wx.EXPAND)
        self.AddLinearSpacer(logs_hSizer, 5)
        
        
        #---- Message sending
        self.tc_msgInput = wx.TextCtrl(self, value="Test message", style=wx.TE_PROCESS_ENTER)
        self.tc_msgInput.Bind(wx.EVT_KEY_DOWN, self.OnEnter)
        btn_sendMsg = wx.Button(self, label="Send Message")
        btn_sendMsg.Bind(wx.EVT_BUTTON, self.OnSendMsg)
        
        sendMsg_hSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(sendMsg_hSizer, 5)
        sendMsg_hSizer.Add(self.tc_msgInput, proportion=1)
        sendMsg_hSizer.Add(btn_sendMsg)
        self.AddLinearSpacer(sendMsg_hSizer, 5)
        
        
        #---- File logging
        btn_log2file = wx.Button(self, label="Log to file")
        btn_log2file.Bind(wx.EVT_BUTTON, self.OnLog2File)
        self.st_filepath = wx.StaticText(self, label="No file selected")
        
        log2file_hSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.AddLinearSpacer(log2file_hSizer, 5)
        log2file_hSizer.Add(btn_log2file)
        self.AddLinearSpacer(log2file_hSizer, 2)
        log2file_hSizer.Add(self.st_filepath, flag=wx.ALIGN_CENTER)
        self.AddLinearSpacer(log2file_hSizer, 5)
        
        
        #---- Main vertical sizer
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(logs_hSizer, proportion=1, flag=wx.EXPAND)
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(sendMsg_hSizer, flag=wx.EXPAND)
        self.AddLinearSpacer(mainSizer, 5)
        mainSizer.Add(log2file_hSizer)
        self.AddLinearSpacer(mainSizer, 5)
        
        self.SetSizer(mainSizer)    # Associate sizer to wx.Panel control
        self.Layout()
    
    def AddLinearSpacer(self, boxsizer, pixelSpacing):
        """A one-dimensional spacer along only the major axis for any BoxSizer"""
        orientation = boxsizer.GetOrientation()
        if orientation == wx.HORIZONTAL:
            boxsizer.Add( (pixelSpacing, 0) )
        elif orientation == wx.VERTICAL:
            boxsizer.Add( (0, pixelSpacing) )
   
    def OnEnter(self, event):
        """Handle when the return key is pressed within tc_msgInput field"""
        key = event.GetKeyCode()
        if key == wx.WXK_RETURN:
            self.OnSendMsg(event)   # Pass off to OnSendMsg function
        event.Skip()
    
    def OnSendMsg(self, event):
        """Called when the send button is pressed"""
        message = self.tc_msgInput.Value  # Get string from input field
        if message: # Only send if non-empty string
            try:
                print "Sending to {}:{} ...".format(remote_host, remote_port),
                client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client.connect(remote_socket)
                client.send(message)
                print "Sent"
                client.shutdown(socket.SHUT_RDWR)
                client.close()
                self.tc_msgInput.Clear()  # Clear text input field after sending successfully
            except socket.error as e:
                print "Socket error: {}".format(e)
    
    def OnClear(self, event):
        """Called when the clear logs button is pressed"""
        self.tc_logDisplay.Clear()
    
    def UpdateDisplay(self, data):
        """Called when the window panel receives an event from pub.sendMessage with data"""
        self.tc_logDisplay.AppendText( str(data) )
    
    def OnLog2File(self, event):
        """Called when the Log to File button is pressed"""
        dlg = wx.FileDialog(self,
                            message="Save As",
                            defaultDir=os.getcwd(),
                            wildcard="Log files (*.log)|*.log",
                            style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
        result = dlg.ShowModal()
        
        if result == wx.ID_OK:
            path = dlg.GetPath()
            self.st_filepath.SetLabel(str(path))   # Update file path text label
            
            self.filepath = path    # Used by WriteFile
            pub.subscribe(self.WriteFile, 'update') # Enable call to WriteFile when receiving new subscriber data
        
        dlg.Destroy()
    
    def WriteFile(self, data):
        try:
            f = open(self.filepath, 'a')
            #print "Opened file {} in append mode".format(f.name)    # Verbose debug
            try:
                #print "Writing \"{}\" to file {}".format(data, f.name)    # Verbose debug
                f.write( str(data) )
            except IOError as e:
                print "File write error: {}".format(e)
            finally:
                    f.close()
                    #print "File closed"    # Verbose debug
            
        except IOError as e:
                print "File IO error: {}".format(e)
                sys.exit(1)


class MainWindow(wx.Frame):
    def __init__(self, parent, title, size=(800, 500)):
        wx.Frame.__init__(self, parent=parent, title=title, size=size)
        
        self.Auth()
        
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        
        framePanel = wx.Panel(self)
        framePanel.BackgroundColour = (235, 230, 220)   # Your neutralness, it's a beige alert!
        
        logPanel = LoggingPanel(parent=framePanel, style=wx.BORDER_SUNKEN)
        nodeEmuPanel = NodeEmuPanel(parent=framePanel, style=wx.BORDER_SUNKEN)
        
        mainSizer = wx.BoxSizer(wx.HORIZONTAL)
        mainSizer.Add(logPanel, proportion=1, flag=wx.EXPAND)
        mainSizer.Add(nodeEmuPanel, proportion=0.8, flag=wx.EXPAND)
        mainSizer.Layout()
        
        framePanel.SetSizer(mainSizer)
        
        #self.Center()
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
    
    def Auth(self):
        """Authenticate with a simple password dialog"""
        secret = hashlib.sha224('thepassword').hexdigest()  # Hash the secret password
        attempts = 1
        maxattempts = 3
        
        dlg = wx.TextEntryDialog(self, message="Enter password", caption="Auth", style=wx.OK|wx.PASSWORD)
        dlg.ShowModal()
        password = hashlib.sha224(dlg.GetValue()).hexdigest()
        
        while not password == secret and attempts < maxattempts:
            attempts += 1
            dlg.SetValue("")
            dlg.ShowModal()
            password = hashlib.sha224(dlg.GetValue()).hexdigest()
        
        if attempts > maxattempts-1:
            wx.MessageDialog(self,
                             "Max number of password attempts ({}) has been reached".format(maxattempts),
                             style=wx.OK).ShowModal()
            sys.exit(0)
        
        dlg.Destroy()


# Used by node emulation buttons to send messages over serial
# TODO: Change this into something that doesn't rely on a global variable
global serialObj

if __name__ == "__main__":
    app = wx.App(False)
    
    # Start serial thread which will monitor serial port
    # and send data to the text display on the GUI window
    serialObj = SerialClient(port='COM11', baudrate=9600)
    
    #local_socket = ('localhost', 8081); remote_socket = ('localhost', 8082) # For local client/server testing
    frame = MainWindow(None, title="Serial and Socket Logger 1")
    
    #local_socket = ('localhost', 8082); remote_socket = ('localhost', 8081) # For local client/server testing
    #frame2 = MainWindow(None, title="Serial and Socket Logger 2")
    
    app.MainLoop()
