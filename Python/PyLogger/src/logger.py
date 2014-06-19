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
import wx
from wx.lib.pubsub import pub

local_host = socket.gethostbyname(socket.getfqdn()) # Use local interface IP address
local_port = 31000#random.randint(1025,36000) # Choose a random unprivileged port
remote_host = '142.156.193.157'
remote_port = 31001

local_socket = (local_host, local_port)
remote_socket = (remote_host, remote_port)


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
        self.reader()
    
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
        print "Listening on {}:{}".format(local_host, local_port)
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
                print "Sending to {}:{}".format(remote_host, remote_port)
                client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client.connect(remote_socket)
                client.send(message)
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
                            message="Save as...",
                            defaultDir=os.getcwd(),
                            wildcard="Log files (*.log)|*.log",
                            style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
        result = dlg.ShowModal()
        if result == wx.ID_OK:
            path = dlg.GetPath()
            self.st_filepath.SetLabel(str(path))   # Update file path text label
        dlg.Destroy()


class MainWindow(wx.Frame):
    def __init__(self, parent, title, size=(500, 500)):
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
    
    local_socket = ('localhost', 8081); remote_socket = ('localhost', 8082)
    frame = MainWindow(None, title="Serial and Socket Logger 1")
    
    #local_socket = ('localhost', 8082); remote_socket = ('localhost', 8081)
    #frame2 = MainWindow(None, title="Serial and Socket Logger 2")
    
    # Start serial thread which will monitor serial port
    # and send data to the text display on the GUI window
    #serial = SerialClient(port='COM11', baudrate=9600)
    
    app.MainLoop()
