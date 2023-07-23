import panel as pn
import json
from panel.template import MaterialTemplate, DarkTheme

protectedKeys = [
    "discordBotToken",
    "inviteCodesWebHook"
]

with open("custom.css", "r") as f:
    css = f.read() 

pn.extension(raw_css=[css], notifications=True)

with open("ccgm_config.json", "r") as f:
    ccgmConfig = json.load(f)

def render_editor():
    json_editor = pn.widgets.JSONEditor(
        value=ccgmConfig, 
        sizing_mode="stretch_both",
        width_policy="min",
        min_width=400,
        min_height=400,
        mode="tree",
        css_classes=[]
    )

    def save_config(event):
        with open("ccgm_config.json", "w") as f:
            json.dump(json_editor.value, f, indent=1)

        pn.state.notifications.success("Configuration saved!")

    save_button = pn.widgets.Button(name="Save", button_type="primary", width=200)
    save_button.on_click(save_config)

    gridbox = pn.GridBox(
        json_editor,
        pn.layout.Row(save_button),
        sizing_mode="stretch_both",
        ncols=1,
        align="center",
        css_classes=["ccgm-grid-box"]
    )
    
    template = MaterialTemplate(theme="dark")

    template.title = "CCGM Config Editor"
    template.header_color = "#ff0000"
    template.main.append(gridbox)

    return template