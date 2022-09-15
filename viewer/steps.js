const ARMY_WIDTH = 3;
const HALF_BOARD_WIDTH = ARMY_WIDTH + 2;
const FULL_BOARD_WIDTH = 2* HALF_BOARD_WIDTH;

function discard(name){
	let allResources = [...resourceLists[iActive].children];
	let allSuitableResources = allResources.filter( el => (el.textContent == name ) );
	allSuitableResources[0].remove();
}

function draw(name){
	console.log("draw(" + name + ")");
	let resource = document.createElement('li');

	resource.textContent = name;
	console.log(resource.textContent);
	resourceLists[iActive].appendChild(resource);	
}

function teamClass(iTeam){
	return 'teamid-' + teamNames[iTeam].toLowerCase().replaceAll(' ', '-')
}

function characterFactory(char, iTeam){
	let charDom = document.createElement('div');
	charDom.id = 'cid-' + char.cid;
	charDom.classList.add( teamClass(iTeam) );
	charDom.classList.add('character');
	charDom.classList.add('team-' + (iTeam+1));
	charDom.innerHTML = 
		`<h1>${char.name}</h1><div class="stats"><div class="hp">${char.hp}</div><div class="maxHP">${char.maxHP}</div>`+
		`<div class="atk softAtk">${char.softAtk}</div><div class="atk hardAtk">${char.hardAtk}</div><div class="mov">${char.mov}</div><div class="rng">${char.rng}</div>`+
		`<div class="netWorth">${char.netWorth}</div><i>${char.flavor}</i>`;
	return charDom;
}

function bluePrintCharacter(charDom){
	let stats = charDom.children[1];
	return {
		hp : Number( stats.children[0].textContent ),
		maxHP : Number( stats.children[1].textContent ),
		softAtk : Number( stats.children[2].textContent ),
		hardAtk : Number( stats.children[3].textContent ),
		mov : Number( stats.children[4].textContent ),
		rng : Number( stats.children[5].textContent ),
		netWorth : Number( stats.children[6].textContent ),
		flavor : stats.children[7].textContent
	};
}

function apply(step, backwards) {
	console.log(step);
	switch(step.action){
		case 'beginturn': return { 'action' : 'beginturn' };
		case 'pass': return { 'action' : 'pass' };
		case 'move': {
			let fieldFrom;
			let fieldTo;
			let unit;
			let moves = step.moves;
			for(let i = 0; i < moves.length; i++) {
				switch (moves[i].toLowerCase()) {
					case "right":
						fieldFrom = boardDom.children[ step.frm[1] + FULL_BOARD_WIDTH*step.frm[0] ];
						fieldTo = boardDom.children[ step.frm[1] + 1 + FULL_BOARD_WIDTH*step.frm[0] ];
						unit = fieldFrom.firstChild;
						if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
						if(!fieldTo.hasChildNodes()) {
							fieldTo.appendChild(unit);
						} else {
							fieldTo.appendChild(unit);
							fieldFrom.appendChild(fieldTo.firstChild);
						}

						step.frm[1]++;
						break;
					case "left":
						fieldFrom = boardDom.children[ step.frm[1] + FULL_BOARD_WIDTH*step.frm[0] ];
						fieldTo = boardDom.children[ step.frm[1] - 1 + FULL_BOARD_WIDTH*step.frm[0] ];
						unit = fieldFrom.firstChild;
						if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
						fieldTo.appendChild(unit);
						fieldFrom.appendChild(fieldTo.firstChild);
						step.frm[1]--;
						break;
					case "down":
						fieldFrom = boardDom.children[ step.frm[1] + FULL_BOARD_WIDTH*step.frm[0] ];
						fieldTo = boardDom.children[ step.frm[1] + FULL_BOARD_WIDTH ];
						unit = fieldFrom.firstChild;
						if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
						fieldTo.appendChild(unit);
						fieldFrom.appendChild(fieldTo.firstChild);
						step.frm[0] = 1;
						break;
					case "up":
						fieldFrom = boardDom.children[ step.frm[1] + FULL_BOARD_WIDTH*step.frm[0] ];
						fieldTo = boardDom.children[ step.frm[1] ];
						unit = fieldFrom.firstChild;
						if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
						fieldTo.appendChild(unit);
						fieldFrom.appendChild(fieldTo.firstChild);
						step.frm[0] = 0;
						break;
				}
			}
			return { 'action' : 'move', 'target' : step.target, 'frm' : step.to, 'to' : step.frm, 'firstCOF' : step.firstCOF };
		}
		case 'draw': {
			let textContent = step.clss + ' - ' + step.value;
			if(!backwards) draw(textContent);
			else discard(textContent);
			return step;
		}
		case 'atk': {
			if(!backwards){
				discard('action - ' + step.cardLost.toUpperCase());

				let victim = boardDom.children[ step.object[1] + FULL_BOARD_WIDTH*step.object[0] ].firstChild;
				victim.children[1].children[0].textContent = step.setLife;
				if(step.delete){
					step.deleted = bluePrintCharacter(victim);
					victim.remove();
				}
				iActive = 1 - iActive;
				return step;
			} else {
				iActive = 1 - iActive;
				let charDom = undefined;
				if(step.delete){
					charDom = characterFactory(step.deleted, 1 - iActive);
					boardDom.children[ step.object[1] + FULL_BOARD_WIDTH*step.object[0] ].appendChild(charDom);
					delete step.deleted;
				} else {
					charDom = boardDom.children[ step.object[1] + FULL_BOARD_WIDTH*step.object[0] ].firstChild;
				}
				charDom.children[1].children[0].textContent = step.setLife + step.lostLife;
				draw('action - ' + step.cardLost.toUpperCase());
				return step;
			}
		}
		case 'def': {
			let subject = boardDom.children[ step.subject[1] + FULL_BOARD_WIDTH*step.subject[0] ];
			if(!backwards){
				discard('action - ' + step.cardLost.toUpperCase());
				subject.children[0].children[1].children[0].textContent = step.permanent;
				iActive = 1 - iActive;
			} else {
				iActive = 1 - iActive;
				subject.children[0].children[1].children[0].textContent = step.permanent - 50;
				draw('action - ' + step.cardLost.toUpperCase());
			}
			return step;
		}
		default:
			console.error('Step type ' + step.action + ' is not implemented');
	}
}

var teamNames = [];
var boardDom = [];
var resourceLists = [];
var screen = null;
var iActive = 0;

function createState(teams, names, board){
	for(let team of teams)
		for(let row of team)
				if(!row.hp) row.hp = row.maxHP;

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
		for(let j = 0; j < FULL_BOARD_WIDTH; j++){
			let td = document.createElement('div');
			td.classList.add('board-field');
			boardDom.appendChild(td);
			row.push(td);
		}
	}

	let headers = [];

	for(let i=0; i<2; i++){
		teamNames[i] = names[i];

		let header = document.createElement('div');
		header.innerHTML = `<h3>${names[i].name}</h3>`;
		let resourceList = document.createElement('ul');
		resourceList.classList.add('resource-list');
		resourceLists.push(resourceList);
		header.appendChild(resourceList);
		header.style.cssText += 'flex:1;'
		header.classList.add(teamClass(i));
		header.classList.add('header');
		header.classList.add('teamIndex-' + i);
		headers.push(header);

		for(let j=0; j<2; j++) {
			let team = teams[i];
			let pos = board[j];
			if (i == 0) {
				for (let k = 0; k < HALF_BOARD_WIDTH; k++) {
					let poss = pos[k];
					let charDom;
					if(poss != null) {
						let posChar = poss[1];
						charDom = characterFactory(team[posChar], i);
						boardDom.children[k + FULL_BOARD_WIDTH * j].appendChild(charDom);
					}
				}
			} else {
				for (let k = HALF_BOARD_WIDTH; k < FULL_BOARD_WIDTH; k++) {
					let charDom;
					let poss = pos[k];
					if(poss != null) {
						let posChar = poss[1];
						charDom = characterFactory(team[posChar], i);
						boardDom.children[k + FULL_BOARD_WIDTH * j].appendChild(charDom);
					}
				}
			}
		}
	}

	screen.prepend(headers[0]);
	screen.append(headers[1]);

	return null;
}

function readGame(content){
	let ret = JSON.parse(content);
	let teams = ret.state.aliveUnits;
	let names = ret.game.teamNames;
	let board = ret.state.board;
	let state = createState(teams, names, board);

	return { state : state, steps : ret.steps }
}

function readStep(content) {
	return JSON.parse(content);
}
