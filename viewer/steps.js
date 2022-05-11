function invert(step, state, backwards) {
	switch(step.action){
		case 'pass': return { 'action' : 'pass' };
		case 'move': return { 'action' : 'move', 'target' : step.target, 'from' : step.to, 'to' : step.from, 'isCOF' : step.isCOF };
	}
}
function apply(step, backwards) {
	switch(step.action){
		case 'pass': return;
		case 'move': {
			let fieldFrom = boardDom.children[ step.from[0] + 12*step.from[1] ];
			let fieldTo = boardDom.children[ step.to[0] + 12*step.to[1] ];
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
				let char = document.createElement('div');
				char.id = 'cid-' + teams[i].characters[j][k].cid;
				char.classList.add(teamClass);
				char.classList.add('character');
				char.classList.add('team-' + (i+1));
				char.innerHTML = teams[i].characters[j][k].name;
				boardDom.children[k + 1 + 6*i + 12*j].appendChild(char);
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
