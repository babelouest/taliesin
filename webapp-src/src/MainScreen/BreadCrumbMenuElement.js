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
	
	handleGotoPath(path) {
		StateStore.dispatch({ type: 'setCurrentPath', path: path });
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
