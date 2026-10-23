import subprocess,json,collections,sys
# Count newline bytes in non-NUL blobs; source suffixes are a heuristic.
source_suffixes={"c","h","cc","cpp","cxx","hh","hpp","s","S","asm","lua","sh","bash","csh","f","f90","y","l","rs","py","pl","pm","awk"}
refs=['release/15.1.0^{commit}','e38a7085f3a8','18799602c9b0','HEAD']
trees={}
for ref in refs:
 rev=subprocess.check_output(['git','rev-parse',ref],text=True).strip()
 entries=[]
 for row in subprocess.check_output(['git','ls-tree','-r','-l','-z',rev]).split(b'\0'):
  if not row:continue
  meta,path=row.split(b'\t',1);mode,kind,oid,size=meta.split()
  if kind==b'blob':entries.append((path.decode(),oid.decode(),int(size)))
 trees[ref]=(rev,entries)
oids={oid for rev,entries in trees.values() for path,oid,size in entries}
p=subprocess.Popen(['git','cat-file','--batch'],stdin=subprocess.PIPE,stdout=subprocess.PIPE)
counts={}
for oid in oids:
 p.stdin.write((oid+'\n').encode());p.stdin.flush()
 header=p.stdout.readline().split();size=int(header[2]);data=p.stdout.read(size);p.stdout.read(1)
 counts[oid]=None if b'\0' in data else data.count(b'\n')
p.stdin.close();p.wait()
results={}
for ref,(rev,entries) in trees.items():
 groups=collections.defaultdict(lambda:dict(files=0,bytes=0,text_lines=0,binary_files=0))
 for path,oid,size in entries:
  for key in ['TOTAL',path.split('/')[0] if '/' in path else '(root)'] + (['SOURCE_SUFFIXES'] if path.rsplit('.',1)[-1] in source_suffixes else []):
   g=groups[key];g['files']+=1;g['bytes']+=size
   if counts[oid] is None:g['binary_files']+=1
   else:g['text_lines']+=counts[oid]
 results[ref]={'commit':rev,'groups':dict(groups)}
with open(sys.argv[1] if len(sys.argv)>1 else 'halfbsd-reduction.json','w') as f:json.dump(results,f,indent=2)
for ref,r in results.items():print(ref,r['commit'],r['groups']['TOTAL'])
