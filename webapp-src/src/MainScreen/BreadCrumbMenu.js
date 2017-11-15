import React, { Component } from 'react';
import { Breadcrumb } from 'react-bootstrap';
import StateStore from '../lib/StateStore';
import BreadCrumbMenuElement from './BreadCrumbMenuElement';

class BreadCrumbMenu extends Component {
  constructor(props) {
    super(props);
		this.state = {dataSource: props.dataSource, path: props.path};
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({dataSource: nextProps.dataSource, path: nextProps.path})
	}
	
	handleGotoRoot() {
		StateStore.dispatch({ type: 'setCurrentPath', path: "" });
	}
	
	render() {
		var breadcrumbList = [];
		var path = "";
		this.state.path.split("/").forEach((subPath) => {
			path += (subPath + "/");
			if (subPath) {
				breadcrumbList.push(
					<BreadCrumbMenuElement path={path} subPath={subPath} key={path}/>
				);
			}
		});
		
		return (
			<Breadcrumb>
				<Breadcrumb.Item onClick={() => this.handleGotoRoot()}>
					{this.state.dataSource}
				</Breadcrumb.Item>
				{breadcrumbList}
			</Breadcrumb>
		);
	}
}

export default BreadCrumbMenu;
