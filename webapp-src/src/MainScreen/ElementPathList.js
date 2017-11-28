import React, { Component } from 'react';
import { Row, Col } from 'react-bootstrap';
import StateStore from '../lib/StateStore';
import ElementButtons from './ElementButtons';
import ModalMedia from '../Modal/ModalMedia';

class ElementPathList extends Component {
  constructor(props) {
    super(props);
		this.state = {
			dataSource: props.dataSource, 
			path: props.path, 
			element: props.element, 
			show: false, 
			modalMedia: false, 
			modalTitle: false
		};
		
		this.handleChangePath = this.handleChangePath.bind(this);
		this.handleDetailsFile = this.handleDetailsFile.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSource: nextProps.dataSource, 
			path: nextProps.path, 
			element: nextProps.element, 
			show: false, 
			modalMedia: false, 
			modalTitle: false
		});
	}
	
	handleChangePath(name) {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.dataSource})});
		StateStore.dispatch({type: "setCurrentPath", path: this.state.path + "/" + name});
	}
	
	handleDetailsFile(name) {
		var modalMedia = this.state.element;
		modalMedia.data_source = this.state.dataSource;
		this.setState({show: true, modalMedia: modalMedia, modalTitle: this.state.element.name});
	}
	
	render() {
		var element;
		if (this.state.element.type === "folder") {
			element = <a role="button" onClick={() => this.handleChangePath(this.state.element.name)}>{this.state.element.name}</a>;
		} else {
			element = <a role="button" onClick={() => this.handleDetailsFile(this.state.element.name)}>{this.state.element.name}</a>;
		}
		return (
			<Row>
				<Col md={8} sm={8} xs={6}>
					{element}
				</Col>
        <Col md={4} sm={4} xs={6} className="text-right">
          <ElementButtons dataSource={this.state.dataSource} path={this.state.path + "/" + this.state.element.name} element={this.state.element}/>
        </Col>
				<ModalMedia show={this.state.show} media={this.state.modalMedia} title={this.state.modalTitle} />
			</Row>
		);
	}
}

export default ElementPathList;
