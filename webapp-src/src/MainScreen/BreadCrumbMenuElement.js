import React, { Component } from 'react';
import { Breadcrumb } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class BreadCrumbMenuElement extends Component {
	constructor(props) {
		super(props);
		this.state = {subPath: props.subPath, path: props.path};
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({subPath: nextProps.subPath, path: nextProps.path})
	}
	
	trimSlashes(str) {
		return str.replace(new RegExp(
			"^[/]+|[/]+$", "g"
		), "");
	}
	
	handleGotoPath(path) {
		StateStore.dispatch({ type: 'setCurrentPath', path: this.trimSlashes(path) });
	}

	render() {
		return (
			<Breadcrumb.Item onClick={() => this.handleGotoPath(this.state.path)}>
				{this.state.subPath}
			</Breadcrumb.Item>
		);
	}
}

export default BreadCrumbMenuElement;
