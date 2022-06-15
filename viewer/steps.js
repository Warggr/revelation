const ARMY_WIDTH = 3;
const HALF_BOARD_WIDTH = ARMY_WIDTH + 2;
const FULL_BOARD_WIDTH = 2* HALF_BOARD_WIDTH;

function discard(name){
	let allResources = [...resourceLists[iActive].children];
	let allSuitableResources = allResources.filter( el => (el.textContent == name ) );
	allSuitableResources[0].remove();

	let discardedCard = document.createElement('span');
	discardedCard.innerHTML = name;
	document.getElementsByClassName('action-deck')[iActive].children[2].appendChild(discardedCard);
}

function draw(name){
	let resource = document.createElement('li');
	resource.textContent = name;
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
		`<h1>${char.name}</h1><div class="stats"><div class="HP">${char.HP}</div><div class="maxHP">${char.maxHP}</div>`+
		`<div class="atk softAtk">${char.softAtk}</div><div class="atk hardAtk">${char.hardAtk}</div><div class="mov">${char.mov}</div><div class="rng">${char.rng}</div>`+
		`<div class="netWorth">${char.netWorth}</div><i>${char.flavor}</i>`;
	return charDom;
}

function bluePrintCharacter(charDom){
	let stats = charDom.children[1];
	return {
		HP : Number( stats.children[0].textContent ),
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
	switch(step.action){
		case 'pass': return { 'action' : 'pass' };
		case 'move': {
			let fieldFrom = boardDom.children[ step.frm[1] + FULL_BOARD_WIDTH*step.frm[0] ];
			let fieldTo = boardDom.children[ step.to[1] + FULL_BOARD_WIDTH*step.to[0] ];
			let unit = fieldFrom.firstChild;
			if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
			fieldTo.appendChild(unit);
			if(step.isCOF){
				fieldFrom.appendChild(fieldTo.firstChild);
			}

			return { 'action' : 'move', 'target' : step.target, 'frm' : step.to, 'to' : step.frm, 'isCOF' : step.isCOF };			
		}
		case 'draw': {
			let deck = document.getElementsByClassName('action-deck')[iActive];
			console.log(deck.children);
			let oldDeckConfig = [ deck.children[0].textContent, deck.children[2].childElementCount ];
			let textContent = step.clss + ' - ' + step.value;

			if(!backwards) draw(textContent);
			else discard(textContent);

			deck.children[0].textContent = step.newDeckSize[0];
			if(step.newDeckSize[1] == 0) deck.children[2].innerHTML = '';
			if(step.newDeckSize[1] != deck.children[2].childElementCount) console.log(`Warning: Discarded card count does not match (${step.newDeckSize[1]} required, ${deck.children[2].childElementCount} found)`);
			step.newDeckSize = oldDeckConfig;
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
				subject.children[1].children[0].textContent = step.permanent;
				iActive = 1 - iActive;
			} else {
				iActive = 1 - iActive;
				subject.children[1].children[0].textContent = step.permanent - 50;
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
var actionDecks = [];
var screen = null;
var iActive = 0;

function createState(state){
	/* Initialize the screen */
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

	/* Initialize the board */
	for(let i = 0; i < 2; i++){
		//let row = [];
		for(let j = 0; j < FULL_BOARD_WIDTH; j++){
			let td = document.createElement('div');
			td.classList.add('board-field');
			//row.push(td);
			if(state.board[i][j]){
				let charDom = characterFactory( state.board[i][j], (j < HALF_BOARD_WIDTH ? 0 : 1) );
				td.appendChild(charDom);
			}
			boardDom.appendChild(td);
		}
	}

	/* Initialize the headers */
	let headers = [];

	for(let i=0; i<2; i++){
		let header = document.createElement('div');
		header.innerHTML = `<h3>${teamNames[i]}</h3>`;
		header.style.cssText += 'flex:1;'
		header.classList.add(teamClass(i));
		header.classList.add('header');
		header.classList.add('teamIndex-' + i);
		headers.push(header);

		let resourceList = document.createElement('ul');
		resourceList.classList.add('resource-list');
		resourceLists.push(resourceList);
		header.appendChild(resourceList);

		let actionDeck = document.createElement('div');
		actionDeck.classList.add('action-deck');
		actionDeck.innerHTML = `<span>${ state.players[i].actionDeckSize[0] }</span>cards<br/>Discard pile:<ul></ul>`;
		header.appendChild(actionDeck);
	}

	screen.prepend(headers[0]);
	screen.append(headers[1]);

	let resourceDeck = document.createElement('div');
	resourceDeck.id = 'resource-deck';
	resourceDeck.innerHTML = `<h3>Resource deck</h3><span>${ state.resourceDeck[0] }</span> cards<br/>Discard pile:<ul></ul>`;
	screen.prepend(resourceDeck);

	return null;
}

function readGame(content){
	let ret = JSON.parse(content);
	console.log(ret);
	teamNames = ret.game.teamNames;
	let state_summary = ret.state;
	let state = createState(state_summary);

	return { state : state, steps : ret.steps }
}

function readStep(content) {
	return JSON.parse(content);
}
