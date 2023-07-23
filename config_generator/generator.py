import panel as pn
from main import render_editor
import webview

app = render_editor()
server = pn.serve(app, show=False, port=5554, threaded=True)
webview.create_window('CCGM Config Editor', "http://localhost:5554", width=800, height=800)
webview.start()