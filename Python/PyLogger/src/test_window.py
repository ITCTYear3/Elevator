'''
wxPython test window layout
'''

import wx


class MainWindow(wx.Frame):
    def __init__(self, parent, title):
        super(MainWindow, self).__init__(parent, title=title)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.InitUI()
        self.Center()
        self.Show()
    
    def InitUI(self):
        panel = wx.Panel(self)
        
        vbox = wx.BoxSizer(wx.VERTICAL)
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.textTitle = wx.StaticText(panel, label="Text area")
        hbox1.Add(self.textTitle)
        vbox.Add(hbox1, flag=wx.LEFT|wx.TOP)
        
        hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.textDisplay = wx.TextCtrl(panel, value="default text", style=wx.TE_MULTILINE)
        hbox2.Add(self.textDisplay, 1, flag=wx.EXPAND|wx.ALL, border=5)
        vbox.Add(hbox2, flag=wx.EXPAND)
        
        hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        btn1 = wx.Button(panel, label="Btn1")
        btn1.Bind(wx.EVT_BUTTON, self.onBtn1("Button 1 press"))
        btn2 = wx.Button(panel, label="Btn2")
        btn3 = wx.Button(panel, label="Btn3")
        hbox3.Add(btn1)
        hbox3.Add(btn2)
        hbox3.Add(btn3)
        vbox.Add(hbox3, flag=wx.CENTER)
        
        panel.SetSizer(vbox)
    
    def OnClose(self, event):
        """Called when the close button is pressed"""
        dlg = wx.MessageDialog(self,
                               "Are you sure you want to close the application?",
                               "Confirmation",
                               wx.YES_NO | wx.NO_DEFAULT | wx.ICON_QUESTION)
        result = dlg.ShowModal()
        dlg.Destroy()
        if result == wx.ID_YES:
            self.Destroy()
    
    def onBtn1(self, msg):
        self.textDisplay.AppendText( "\n" + str(msg) + "\n" )
    


if __name__ == "__main__":
    app = wx.App(False)
    frame = MainWindow(None, title="test.py")
    app.MainLoop()
