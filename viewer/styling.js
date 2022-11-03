function teamClass(teamName){
	return 'teamid-' + teamName.toLowerCase().replaceAll(' ', '-')
}

function characterFactory(char, iTeam, teamName){
	var charDom = document.createElement('div');
	charDom.id = 'cid-' + char.cid;
	charDom.classList.add( teamClass(teamName) );
	charDom.classList.add('character');
	charDom.classList.add('team-' + (iTeam+1));
	charDom.innerHTML =
		`<h1>${char.name}</h1>
		 <div class="stats">
		 <div class="HP">${char.HP}</div><div class="maxHP">${char.maxHP}</div>`+
		`<div class="atk softAtk">${char.softAtk}</div><div class="atk hardAtk">${char.hardAtk}</div><div class="mov">${char.mov}</div><div class="rng">${char.rng}</div>`+
		`<div class="netWorth">${char.netWorth}</div><i>${char.flavor}</i>`;
	return charDom;
}
