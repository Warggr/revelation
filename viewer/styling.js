function teamClass(teamName){
	return 'teamid-' + teamName.toLowerCase().replaceAll(' ', '-')
}

class Character extends React.Component {
	render(){
		let className = 'character';
		if(this.props.team.i != undefined) className += ` team-${this.props.team.i+1}`;
		if(this.props.team.name) className += ` ${teamClass(this.props.team.name)}`;
		return e('div', {
			id : 'cid-' + this.props.char.cid,
			className: className
		}, [
			e('h1', {key:'h1'}, this.props.char.name),
			e('div', { className: "stats", key:'stats' }, [
				e('div', {className:"HP", key:"HP"}, [
					e('span', {className:"permanent redundant", key:"per"}, this.props.char.HP),
					e('span', {className:"temporary", key:"tmp"}, this.props.char.HP),
				]),
				e('div', {className:"maxHP", key:"maxHP"}, this.props.char.maxHP),
				e('div', {className:"atk softAtk", key:"softAtk"}, this.props.char.softAtk),
				e('div', {className:"atk hardAtk", key:"hardAtk"}, this.props.char.hardAtk),
				e('div', {className:"mov", key:"mov"}, this.props.char.mov),
				e('div', {className:"rng", key:"rng"}, this.props.char.rng),
				e('div', {className:"netWorth", key:"netWorth"}, this.props.char.netWorth),
				e('i', {key:'flavor'}, this.props.char.flavor)
			])
		]);
	}
}
