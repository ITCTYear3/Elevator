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

local_host = 'localhost'
local_port = random.randint(1025,36000) # Choose a random unprivileged port
remote_host = '142.156.193.168'
remote_port = 8082

local_socket = (local_host, local_port)
remote_socket = (remote_host, remote_port)


class SerialClient(threading.Thread):
    """Busy wait watching serial port for new incomming data and pass data to subscriber"""
    def __init__(self, port='COM1', baudrate=9600, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE):
        super(SerialClient, self).__init__()
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
                    print "N: {}".format(n)
                    data = data + self.ser.read(n)   # and get as much as possible
                if data:
                    """NOTE: data size is a max of 8 bytes for each read loop"""
                    print "Data: {}".format(data).encode()
                    wx.CallAfter(pub.sendMessage, 'update', data=data)
            except serial.SerialException as e:
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
        super(SocketClient, self).__init__()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind(local_socket)
        self.socket.listen(5)
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


class MainWindow(wx.Frame):
    def __init__(self, parent, title):
        super(MainWindow, self).__init__(parent, title=title)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.InitUI()
        #self.Center()
        self.Show()
        
        pub.subscribe(self.UpdateDisplay, 'update')
        
        # start thread to listen for incomming socket connections
        self.ipc = SocketClient()
    
    def InitUI(self):
        """Setup window elements"""
        panel = wx.Panel(self)
        
        vbox = wx.BoxSizer(wx.VERTICAL)
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        textTitle = wx.StaticText(panel, label="Logs")
        hbox1.Add(textTitle)
        vbox.Add(hbox1, flag=wx.LEFT|wx.TOP)
        
        hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.textDisplay = wx.TextCtrl(panel, value="", style=wx.TE_MULTILINE|wx.TE_READONLY)
        hbox2.Add(self.textDisplay, 1, flag=wx.EXPAND)
        vbox.Add(hbox2, 1, flag=wx.LEFT|wx.RIGHT|wx.EXPAND, border=5)
        
        hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        self.textInput = wx.TextCtrl(panel, value="Test message", style=wx.TE_LEFT)
        hbox3.Add(self.textInput, 1, flag=wx.ALL, border=5)
        btn1 = wx.Button(panel, label="Send Message")
        btn1.Bind(wx.EVT_BUTTON, self.OnSendMsg)
        hbox3.Add(btn1, flag=wx.CENTER, border=5)
        vbox.Add(hbox3, flag=wx.EXPAND)
        
        panel.SetSizer(vbox)
    
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
    
    def OnSendMsg(self, event):
        """Called when the send button is pressed"""
        message = self.textInput.Value  # Get string from input field
        try:
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.connect(remote_socket)
            client.send(message)
            client.shutdown(socket.SHUT_RDWR)
            client.close()
        except socket.error as e:
            print "Socket error: {}".format(e)
    
    def UpdateDisplay(self, data):
        """Called when the window panel receives an event from pub.sendMessage with data"""
        self.textDisplay.AppendText( str(data) )


if __name__ == "__main__":
    app = wx.App(False)
    
    # First window, override port numbers
    local_socket = (local_host, 8084)
    remote_socket = (local_host, 8085)
    frame = MainWindow(None, title="Serial and Socket Logger 1")
    
    # Second window, override port numbers
    local_socket = (local_host, 8085)
    remote_socket = (local_host, 8084)
    frame2 = MainWindow(None, title="Serial and Socket Logger 2")
    
    # Start serial thread which will monitor serial port
    # and send data to the text display on the GUI window
    serial = SerialClient(port='COM11', baudrate=9600)
    
    app.MainLoop()
