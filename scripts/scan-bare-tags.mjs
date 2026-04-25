import fs from 'node:fs';
import path from 'node:path';

const validHtmlTags = new Set([
  'a','abbr','address','area','article','aside','audio','b','base','bdi','bdo','blockquote','body',
  'br','button','canvas','caption','cite','code','col','colgroup','data','datalist','dd','del',
  'details','dfn','dialog','div','dl','dt','em','embed','fieldset','figcaption','figure','footer',
  'form','h1','h2','h3','h4','h5','h6','head','header','hgroup','hr','html','i','iframe','img',
  'input','ins','kbd','label','legend','li','link','main','map','mark','meta','meter','nav','noscript',
  'object','ol','optgroup','option','output','p','param','picture','pre','progress','q','rp','rt',
  'ruby','s','samp','script','section','select','small','source','span','strong','style','sub',
  'summary','sup','svg','table','tbody','td','template','textarea','tfoot','th','thead','time',
  'title','tr','track','u','ul','var','video','wbr','center','font','strike','tt','big',
  'g','path','rect','circle','line','polyline','polygon','text','defs','use','symbol','clippath',
  'lineargradient','radialgradient','stop','filter','mask','pattern','foreignobject','marker','tspan'
]);

function walk(dir, out){
  for (const e of fs.readdirSync(dir, {withFileTypes:true})){
    const full = path.join(dir,e.name);
    if (e.isDirectory()) walk(full,out);
    else if (e.isFile() && e.name.toLowerCase().endsWith('.md')) out.push(full);
  }
  return out;
}

const fenceRe = /^\s*(```|~~~)/;
const tagRe = /<\/?([a-zA-Z][a-zA-Z0-9_-]*)\b[^>]*>/g;

const files = walk('docs/notes',[]);
const issues = [];
for (const f of files){
  const text = fs.readFileSync(f,'utf8');
  const lines = text.split(/\r?\n/);
  let inFence = false;
  for (let i=0;i<lines.length;i++){
    const ln = lines[i];
    if (fenceRe.test(ln)) { inFence = !inFence; continue; }
    if (inFence) continue;
    const stripped = ln.replace(/`[^`]*`/g,'');
    let m;
    tagRe.lastIndex = 0;
    while((m=tagRe.exec(stripped))){
      const tag = m[1].toLowerCase();
      if (!validHtmlTags.has(tag)) {
        issues.push({file:f, line:i+1, tag:m[0], tagName:tag, raw:ln.trim().substring(0,160)});
      }
    }
  }
}
const byFile = {};
for (const it of issues){ (byFile[it.file]=byFile[it.file]||[]).push(it); }
console.log('Files with bare unknown tags:', Object.keys(byFile).length);
console.log('Total occurrences:', issues.length);
const tagCounts = {};
for (const it of issues) tagCounts[it.tagName] = (tagCounts[it.tagName]||0)+1;
console.log('Top tag names:', Object.entries(tagCounts).sort((a,b)=>b[1]-a[1]).slice(0,30));
console.log();
for (const [f,arr] of Object.entries(byFile)){
  console.log('---', f, '(', arr.length, 'issues )');
  for (const it of arr.slice(0,8)) console.log('  L'+it.line+': '+it.tag+'  | '+it.raw);
  if (arr.length>8) console.log('  ... and',arr.length-8,'more');
}
