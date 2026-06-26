const fs = require('fs');

const contextData = fs.readFileSync('explanation-context.json', 'utf8');
const manifest = fs.readFileSync('game-system.manifest.json', 'utf8');
const syncData = fs.readFileSync('index-sync.json', 'utf8');

const stateJs = fs.readFileSync('simulation-state.js', 'utf8').replace('export class', 'class');
const rendererJs = fs.readFileSync('simulation-renderer.js', 'utf8').replace('export class', 'class');
const entriesJs = fs.readFileSync('simulation-entries.js', 'utf8').replace('export const registry', 'const registry');

let html = fs.readFileSync('index.html', 'utf8');

// Remove import statements
html = html.replace(/import .*?;\r?\n/g, '');
html = html.replace('<script type="module">', '<script>');

// Replace loadData
const loadDataRegex = /async function loadData\(\) \{[\s\S]*?\n        \}/;
const newLoadData = `async function loadData() {
            try {
                buildContextUI();
                renderer.render(simState);
                ui.title.innerText = "준비 완료";
            } catch (err) {
                ui.title.innerText = "데이터 로딩 실패!";
                console.error(err);
                changeState('error');
            }
        }`;
html = html.replace(loadDataRegex, newLoadData);

// Inject variables
const injection = `
        let manifest = ${manifest};
        let contextData = ${contextData};
        let syncData = ${syncData};
        
${stateJs}

${rendererJs}

${entriesJs}
`;

html = html.replace('let manifest, contextData, syncData;', injection);

fs.writeFileSync('index_standalone.html', html, 'utf8');
console.log('Standalone HTML generated successfully.');
