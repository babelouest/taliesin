import React, { Component } from 'react';
import { Row, Col } from 'react-bootstrap';

import ElementButtons from './ElementButtons';
import ModalMedia from '../Modal/ModalMedia';
import StateStore from '../lib/StateStore';

class ElementCategoryList extends Component {
	constructor(props) {
		super(props);
		this.state = {
			dataSource: props.dataSource, 
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue, 
			element: props.element,
			modal: ""
		};
		
		this.handleChangePath = this.handleChangePath.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSource: nextProps.dataSource, 
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue, 
			element: nextProps.element,
			modal: ""
		});
	}
	
	handleChangePath(name) {
		if (this.state.element.type === "media") {
			this.setState({modal: <ModalMedia show={true} media={this.state.element} title={this.state.element.tags.title||this.state.element.name} />});
		} else {
			if (this.state.subCategory) {
                            StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			    StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue, subCategory: this.state.subCategory, subCategoryValue: name});
			} else {
                            StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			    StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: name});
			}
		}
	}
	
	render() {
		return (
			<Row className="row-media">
				<Col md={8} sm={8} xs={6}>
					<a role="button" onClick={() => this.handleChangePath(this.state.element.name)}>{this.state.element.name}</a>
				</Col>
				<Col md={4} sm={4} xs={6} className="text-right">
					<ElementButtons dataSource={this.state.dataSource} path={this.state.element.path} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} element={this.state.element}/>
				</Col>
				{this.state.modal}
			</Row>
		);
	}
}

export default ElementCategoryList;
