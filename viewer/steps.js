function invert(step, state, backwards) {
	switch(step.action){
		case 'pass': return { 'action' : 'pass' };
		case 'move': return { 'action' : 'move', 'target' : step.target, 'frm' : step.to, 'to' : step.frm, 'isCOF' : step.isCOF };
	}
}
function apply(step, backwards) {
	switch(step.action){
		case 'pass': return;
		case 'move': {
			let fieldFrom = boardDom.children[ step.frm[1] + 12*step.frm[0] ];
			let fieldTo = boardDom.children[ step.to[1] + 12*step.to[0] ];
			let unit = fieldFrom.firstChild;
			if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
			fieldTo.appendChild(unit);
			if(step.isCOF){
				fieldFrom.appendChild(fieldTo.firstChild);
			}
		}
	}
}

var boardDom = [];
var screen = null;

function createState(teams){
	let outerScreen = document.getElementById('screen');
	outerScreen.textContent = '';

	screen = document.createElement('div');
	screen.id = 'screen-inner';
	screen.style.cssText += 'width:100%;height:100%;display:flex;flex-direction:column;';
	outerScreen.appendChild(screen);

	boardDom = document.createElement('div');
	boardDom.id = 'board';
	boardDom.style.flex = 2;
	screen.appendChild(boardDom);

	for(let i = 0; i < 2; i++){
		let row = [];
		for(let j = 0; j < 12; j++){
			let td = document.createElement('div');
			td.classList.add('board-field');
			boardDom.appendChild(td);
			row.push(td);
		}
	}

	let headers = [];

	for(let i=0; i<2; i++){
		let teamClass = 'teamid-' + teams[i].name.toLowerCase().replaceAll(' ', '-');

		let header = document.createElement('div');
		header.textContent = teams[i].name;
		header.style.cssText += 'flex:1;'
		header.classList.add(teamClass);
		headers.push(header);

		for(let j=0; j<2; j++)
			for(let k=0; k<4; k++){
				let charDom = document.createElement('div');
				let char = teams[i].characters[j][k];
				charDom.id = 'cid-' + teams[i].characters[j][k].cid;
				charDom.classList.add(teamClass);
				charDom.classList.add('character');
				charDom.classList.add('team-' + (i+1));
				charDom.innerHTML = 
					`<h1>${char.name}</h1><div class="stats"><span>${char.hp}</span> max.<span>${char.maxHP}</span>`+
					`<span>${char.softAtk}</span><span>${char.hardAtk}</span><span>${char.mov} MOV</span><span>${char.rng} RNG</span>`+
					`<span>${char.netWorth}$</span><i>${char.flavor}</i>`;
				boardDom.children[k + 1 + 6*i + 12*j].appendChild(charDom);
		}
	}

	screen.prepend(headers[0]);
	screen.append(headers[1]);

	return null;
}

function readGame(content){
	let ret = JSON.parse(content);
	let teams = ret.teams;
	let state = createState(teams);

	return { state : state, steps : ret.steps }
}

function readStep(content) {
	return JSON.parse(content);
}
