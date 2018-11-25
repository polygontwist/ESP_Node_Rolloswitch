// V25.11.2018
var getpostData =function(url, auswertfunc,POSTdata,noheader,rh){
		var loader,i;
		try {loader=new XMLHttpRequest();}
		catch(e){
				try{loader=new ActiveXObject("Microsoft.XMLHTTP");}
				catch(e){
						try{loader=new ActiveXObject("Msxml2.XMLHTTP");}
						catch(e){loader  = null;}
				}
			}	
		if(!loader)alert('XMLHttp nicht möglich.');
		var jdata=undefined;
		if(POSTdata!=undefined)jdata=POSTdata;//encodeURI
		
		loader.onreadystatechange=function(){
			if(loader.readyState==4)auswertfunc(loader);
			};
		loader.ontimeout=function(e){console.log("TIMEOUT");}
		loader.onerror=function(e){console.log("ERR",e,loader.readyState);}
		
		if(jdata!=undefined){console.log(">>1",url);
				loader.open("POST",url,true);
				if(rh!=undefined){
						for(i=0;i<rh.length;i++){
							loader.setRequestHeader(rh[i].typ,rh[i].val);
						}
				}
				if(noheader!==true){
					//loader.responseType="text";
					loader.setRequestHeader("Content-Type","application/x-www-form-urlencoded");
					//loader.setRequestHeader("Content-Type","text/plain");
					loader.setRequestHeader('Cache-Control','no-cache');
					loader.setRequestHeader("Pragma","no-cache");
					loader.setRequestHeader("Cache-Control","no-cache");
					jdata=encodeURI(POSTdata);//
				}
				loader.send(jdata);console.log(">>3",url);
			}
			else{
				loader.open('GET',url,true);
				loader.setRequestHeader('Content-Type', 'text/plain');
				loader.send(null);
			}
	}
var cE=function(ziel,e,id,cn){
	var newNode=document.createElement(e);
	if(id!=undefined && id!="")newNode.id=id;
	if(cn!=undefined && cn!="")newNode.className=cn;
	if(ziel)ziel.appendChild(newNode);
	return newNode;
	}
var gE=function(id){if(id=="")return undefined; else return document.getElementById(id);}
var addClass=function(htmlNode,Classe){	
	var newClass;
	if(htmlNode!=undefined){
		newClass=htmlNode.className;
 
		if(newClass==undefined || newClass=="")newClass=Classe;
		else
		if(!istClass(htmlNode,Classe))newClass+=' '+Classe;	
 
		htmlNode.className=newClass;
	}			
}
var subClass=function(htmlNode,Classe){
		var aClass,i;
		if(htmlNode!=undefined && htmlNode.className!=undefined){
			aClass=htmlNode.className.split(" ");	
			var newClass="";
			for(i=0;i<aClass.length;i++){
				if(aClass[i]!=Classe){
					if(newClass!="")newClass+=" ";
					newClass+=aClass[i];
					}
			}
			htmlNode.className=newClass;
		}
}
var istClass=function(htmlNode,Classe){
	if(htmlNode.className){
		var i,aClass=htmlNode.className.split(' ');
		for(i=0;i<aClass.length;i++){
				if(aClass[i]==Classe)return true;
		}	
	}		
	return false;
}
var filterJSON=function(s){
	var re=s;
	if(re.indexOf("'")>-1)re=re.split("'").join('"');
	try {re=JSON.parse(re);} 
	catch(e){
		console.log("JSON.parse ERROR:",s,":");
		re={"error":"parseerror"};
		}
	return re;
}
 
	
var rollo433=function(){//lokal ESP_Node_Rolloswitch
	var ESP8266URL="./action?rkey=Q00QFF0F0Q00Q10&rollo=";//
	var URLbefehle={
		"UP":	ESP8266URL+"UP",
		"DOWN":	ESP8266URL+"DOWN",
		"STOP":	ESP8266URL+"STOP"
	}
	var getNodeByAttr=function(node,nodeattribute){//z.B. <div data-option="" /> get Element mit "data-option" 
		var nodelist=[];
		var i,reList,j,childlist;
		if(node.getAttribute!=null){
			if(node.getAttribute(nodeattribute)!=undefined) nodelist.push(node);
		}
		if(node.children!=undefined){
			childlist=node.children;
		}else{
			childlist=node.childNodes;//+navigator.userAgent
		}
		if(childlist!=undefined)
		for(i=0;i<childlist.length;i++){
			reList=getNodeByAttr(childlist[i],nodeattribute);
			if(reList.length>0){
				for(j=0;j<reList.length;j++)nodelist.push(reList[j]);
			}
		}
		return nodelist;
	}
	//Daten laden/senden
	var ini=function(){
		var i,nodelist;		
		nodelist=getNodeByAttr(document,"data-433");	
		for(i=0;i<nodelist.length;i++)nodelist[i].addEventListener('click', buttclick);
	}
	var buttclick=function(e){
		var atr=this.getAttribute("data-433"),json=filterJSON(atr);
		sendebefehl(json,this);
		e.preventDefault();
	}	
	var sendebefehl=function(jsonbefehl,node){
		if(jsonbefehl.action!=undefined){
			var tim=new Date();			
			getpostData(URLbefehle[jsonbefehl.action]+'&time='+tim.getTime(),fresult);
			if(typeof jsonbefehl.dauer==="number"){
				setTimeout( function(){sendebefehl({"action":"STOP"},undefined)} , jsonbefehl.dauer*1000);
			}
		}
	}
	var fresult=function(data){console.log(data);}	
	ini();
}

var timerliste=function(){
	var datei="./timer.txt";
	var dateisysinfo="./data.json";
	var ziel,timerdata=[];
	var savetimeout=undefined;
	var timerlistreload=undefined;
	
	var wochentag=["Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag","Sonntag"];
	
	var uploadtimerdaten=function(s){
		//console.log(">>>",s);
		var id= (new Date()).getTime();
		var header=[];
		header.push({typ:"Content-Type",val:"multipart/form-data; boundary=---------------------------"+id});
		
		var senddata="-----------------------------"+id+"\r\n";//CR LF
		 senddata+="Content-Disposition: form-data; name=\"upload\"; filename=\"timer.txt\"\r\n";
		 senddata+="Content-Type: multipart/form-data; boundary=---------------------------"+id+"\r\n";
		 senddata+="Content-Type: text/plain\r\n";
		 senddata+=s;
		 senddata+="-----------------------------"+id+"--\r\n";
		getpostData("./upload", 
					function(data){// console.log("RE:",data.status,data.statusText);
						getTimerlist();
					},
					senddata,
					true,
					header
			);
	}	
	
	var getTimerlist=function(){
		if(timerlistreload!=undefined)clearTimeout(timerlistreload);
		var i,inp=ziel.getElementsByTagName('input');
		for(i=0;i<inp.length;i++){
			inp[i].disabled=true;
		}
		inp=ziel.getElementsByTagName('select');
		for(i=0;i<inp.length;i++){
			inp[i].disabled=true;
		}
		getpostData(datei,retTimerlist);
	}
	var retTimerlist=function(data){
		//convert text to JSON
		var i,t,ti,tidat,tdat=data.responseText.split('\n').join('').split('\r');
		timerdata=[{aktiv:false,zeitstr:"00:00",tage:0,befehl:"UP",id:"new"}];
		for(i=0;i<tdat.length;i++){
			ti=tdat[i].split('|');//on|07:05|31|UP|t1
			tidat={};
			for(t=0;t<ti.length;t++){
				if(t==0)tidat.aktiv=(ti[t]=="on");
				if(t==1)tidat.zeitstr=ti[t];
				if(t==2)tidat.tage=parseInt(ti[t]);
				if(t==3)tidat.befehl=ti[t];
				if(t==4)tidat.id=ti[t];
			}
			timerdata.push(tidat);
		}
		create();
		timerlistreload=setTimeout(function(){(getTimerlist())},1000*60*5);//alle 5min neuladen
	}
	
	var saveandreload=function(now){
		if(savetimeout!=undefined)clearTimeout(savetimeout);
		//timerdata -> außer.id=="new"
		var i,d,s="";
		for(i=0;i<timerdata.length;i++){
			d=timerdata[i];
			if(d.id!="new" && d.id!="-"){
				if(d.aktiv)
					s+="on";
				else
					s+="off";
				s+="|"+d.zeitstr;
				s+="|"+d.tage;
				s+="|"+d.befehl;
				s+="|"+d.id;
				s+="\r\n"
			}
		}
		if(now)
			uploadtimerdaten(s);
			else
			savetimeout=setTimeout(function(){uploadtimerdaten(s)},2000);
	}
	

	
	var setbit=function(bbyte,bitNr,b_val){
		if(b_val)
			return (bbyte | bitNr);
		else
			return (bbyte ^ bitNr);
	}
	
	var changeinput=function(e){
		var dat=this.data;//.id .lnk
		if(dat.id=="time"){dat.lnk.zeitstr=this.value}
		if(dat.id=="Mo"){dat.lnk.tage=setbit(dat.lnk.tage,1,this.checked)}
		if(dat.id=="Di"){dat.lnk.tage=setbit(dat.lnk.tage,2,this.checked)}
		if(dat.id=="Mi"){dat.lnk.tage=setbit(dat.lnk.tage,4,this.checked)}
		if(dat.id=="Do"){dat.lnk.tage=setbit(dat.lnk.tage,8,this.checked)}
		if(dat.id=="Fr"){dat.lnk.tage=setbit(dat.lnk.tage,16,this.checked)}
		if(dat.id=="Sa"){dat.lnk.tage=setbit(dat.lnk.tage,32,this.checked)}
		if(dat.id=="So"){dat.lnk.tage=setbit(dat.lnk.tage,64,this.checked)}
		if(dat.id=="befehl"){dat.lnk.befehl=this.value}
		if(dat.id=="aktiv"){dat.lnk.aktiv=this.checked}
		//wait xsec
		//console.log("refresh",this.data);
		
		if(timerlistreload!=undefined)clearTimeout(timerlistreload);
		timerlistreload=setTimeout(function(){(getTimerlist())},1000*60*5);//alle 5min neuladen

		
		if(dat.lnk.id!="new"){
			addClass(this,"isedit");
			saveandreload(false);
		}
	}
	var delinput=function(e){
		this.data.lnk.id="-";
		this.data.tr.style.opacity=0.5;
		saveandreload(true);
	}
	var addinput=function(e){
		var newdat= JSON.parse( JSON.stringify(this.data.lnk));
		newdat.id='tn'+timerdata.length;
		timerdata.push(newdat);
		saveandreload(true);
	}
	
	var anzinputs=0;
	var cI=function(ziel,typ,value,title){//create input
		var label;
		var input=cE(ziel,"input");
		input.type=typ;
		if(typ=="checkbox"){
			input.checked=value;
			input.id="cb"+anzinputs;
			label=cE(ziel,"label");
			label.htmlFor=input.id;
			input.dataLabel=label;
		}	
		else
			input.value=value;
		if(title!=undefined)input.title=title;	
		anzinputs++;
		return input;
	}
	var addBefehle=function(node,dat){
		var befehllist=['UP','DOWN','STOP'];
		var i,o;
		for(i=0;i<befehllist.length;i++){
			o=cE(node,"option");
			o.value=befehllist[i];
			o.innerHTML=befehllist[i];
		}
	}
	
	var create=function(){
		var i,t,ti,table,tr,td,s,node,div;
		ziel.innerHTML="";
		
		table=cE(ziel,"table",undefined,"timertab");
		//head
		tr=cE(table,"tr");tr.className="tabTihead";
		td=cE(tr,"th");td.innerHTML="Zeit";
		td=cE(tr,"th");td.innerHTML="Mo";
		td=cE(tr,"th");td.innerHTML="Di";
		td=cE(tr,"th");td.innerHTML="Mi";
		td=cE(tr,"th");td.innerHTML="Do";
		td=cE(tr,"th");td.innerHTML="Fr";
		td=cE(tr,"th");td.innerHTML="Sa";
		td=cE(tr,"th");td.innerHTML="So";
		td=cE(tr,"th");td.innerHTML="Befehl";
		td=cE(tr,"th");td.innerHTML="aktiv";
		td=cE(tr,"td");
		
		for(i=0;i<timerdata.length;i++){
			ti=timerdata[i];
			tr=cE(table,"tr");
			if(ti.id=="new")tr.className="tabTiCreate";
			td=cE(tr,"td");
				node=cI(td,"time",ti.zeitstr,"aktiv");//<input class="anioutline" title="aktiv" type="checkbox">
				node.maxlength=5;
				node.size=5;
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"time"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 1)==1,"Mo");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"Mo"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 2)==2,"Di");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"Di"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 4)==4,"Mi");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"Mi"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 8)==8,"Do");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"Do"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 16)==16,"Fr");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"Fr"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 32)==32,"Sa");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"Sa"};
			td=cE(tr,"td");
				node=cI(td,"checkbox",(ti.tage & 64)==64,"So");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"So"};
			
			td=cE(tr,"td");
				node=cE(td,"select");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"befehl"};
				addBefehle(node,ti);
				node.value=ti.befehl;
				
			td=cE(tr,"td");
				node=cI(td,"checkbox",ti.aktiv,"aktiv");
				node.addEventListener('change',changeinput);
				node.data={"lnk":ti,"id":"aktiv"};
				addClass(node,"booleanswitch");
			
			td=cE(tr,"td");
			if(ti.id=="new"){
				node=cI(td,"button","add","hinzufügen");
				node.className="inpbutt badd";
				node.addEventListener('click',addinput);
				node.data={"lnk":ti,"id":"add"};
			}else{
				node=cI(td,"button","del","löschen");
				node.className="inpbutt bdel";
				node.addEventListener('click',delinput);
				node.data={"lnk":ti,"id":"del","tr":tr};
			}
		}
		
	}
	
	
	var changetimekorr=function(e){
		var val=this.value*60*60;//in sekunden
		//console.log(">>>",val);
		getpostData(dateisysinfo+'?settimekorr='+val,
			function(d){
				//console.log('reload data',d);
				setTimeout(function(){
					getpostData(dateisysinfo,fresultsysinfo);
					}
				,1000);//1 sec warten, bis intern Zeit gesetzt wurde
			}
		);
		//settimekorr, led=on, led=off
	}
	
	var ledswitch=function(e){
		getpostData(dateisysinfo+'?led='+this.data,fresultsysinfo);
		e.preventDefault();
	}
	
	var fresultsysinfo=function(data){
		var ziel=gE('sysinfo'),
			jdat=filterJSON(data.responseText),
			div,node,p,a,s;
		if(ziel){
			ziel.innerHTML="";
						
			div=cE(ziel,"div",undefined,"utctimset");
			div.innerHTML="UTC Zeitunterschied:";
			var val=Math.floor(jdat.datum.timekorr/60/60);
			node=cI(div,"number",val,"Zeitunterschied");
			node.addEventListener('change',changetimekorr);
			node.maxlength=2;
			node.size=2;
			if(val==-1 || val==1)
				node=document.createTextNode(" Stunde");
			else
				node=document.createTextNode(" Stunden");
			div.appendChild(node);

			node=cE(ziel,"p");
			node.innerHTML="lokaltime: "+jdat.lokalzeit;
			
			node=cE(ziel,"p");
			s="";
			if(jdat.datum.day<10)s+="0";
			s+=jdat.datum.day+".";
			if(jdat.datum.month<10)s+="0";
			s+=jdat.datum.month+".";
			node.innerHTML="Datum: "+s+jdat.datum.year+" "+wochentag[jdat.datum.tag];
			
			
			node=cE(ziel,"p");
			node.innerHTML="Sommerzeit: "+jdat.datum.summertime;
			
			node=cE(ziel,"p");
			node.innerHTML="MAC: <span style=\"text-transform: uppercase;\">"+jdat.macadresse+"</span>";
			
			node=document.getElementsByTagName('h1')[0];
			if(node)
				node.innerHTML=jdat.progversion;
			
			//fstotalBytes,fsusedBytes,fsused,progversion,aktionen
			
			p=cE(ziel,"p");
			p.innerHTML="LED "
			p.className="buttons";
			a=cE(p,"a");
			a.href="#";
			a.innerHTML="an";
			a.data='on';
			a.addEventListener('click',ledswitch);
			
			a=cE(p,"a");
			a.href="#";
			a.innerHTML="aus";
			a.data='off';
			a.addEventListener('click',ledswitch);
		}
		//console.log(jdat);
	}
	
	var ini=function(){
		ziel=gE("timersetting");
		if(ziel){
			getTimerlist();
		}
		var z2=gE('sysinfo');
		if(z2){
			getpostData(dateisysinfo,fresultsysinfo);//+'&time='+tim.getTime()
		}
	}
	
	ini();
}


window.addEventListener('load', function (event) {
	var r=new rollo433();
	var tl=new timerliste();
});