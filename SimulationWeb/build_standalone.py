import json, re

with open('explanation-context.json', 'r', encoding='utf-8') as f: contextData = f.read()
with open('game-system.manifest.json', 'r', encoding='utf-8') as f: manifest = f.read()
with open('index-sync.json', 'r', encoding='utf-8') as f: syncData = f.read()
with open('simulation-state.js', 'r', encoding='utf-8') as f: stateJs = f.read().replace('export class', 'class')
with open('simulation-renderer.js', 'r', encoding='utf-8') as f: rendererJs = f.read().replace('export class', 'class')
with open('simulation-entries.js', 'r', encoding='utf-8') as f: entriesJs = f.read().replace('export const registry', 'const registry')
with open('index.html', 'r', encoding='utf-8') as f: html = f.read()

html = re.sub(r"import .*?;\n", "", html)
html = html.replace('<script type="module">', '<script>')

def repl_loadData(m):
    return """async function loadData() {
            try {
                buildContextUI();
                renderer.render(simState);
                ui.title.innerText = "준비 완료";
            } catch (err) {
                ui.title.innerText = "데이터 로딩 실패!";
                console.error(err);
                changeState('error');
            }
        }"""
html = re.sub(r"async function loadData\(\) \{.*?\n        \}", repl_loadData, html, flags=re.DOTALL)

injection = f"""
        let manifest = {manifest};
        let contextData = {contextData};
        let syncData = {syncData};
        
{stateJs}

{rendererJs}

{entriesJs}
"""
html = html.replace("let manifest, contextData, syncData;", injection)

with open('index_standalone.html', 'w', encoding='utf-8') as f:
    f.write(html)
print("Done")
