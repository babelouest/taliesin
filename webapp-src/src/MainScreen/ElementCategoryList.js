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
			element: props.element
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
			element: nextProps.element
		});
	}
	
	handleChangePath(name) {
		if (this.state.subCategory) {
			StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue, subCategory: this.state.subCategory, subCategoryValue: name});
		} else {
			StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: name});
		}
	}
	
	render() {
		return (
			<Row>
				<Col md={8} sm={8} xs={6}>
					<a role="button" onClick={() => this.handleChangePath(this.state.element.name)}>{this.state.element.name}</a>
				</Col>
        <Col md={4} sm={4} xs={6} className="text-right">
          <ElementButtons dataSource={this.state.dataSource} path={false} element={this.state.element}/>
        </Col>
				<ModalMedia show={this.state.show} media={this.state.modalMedia} title={this.state.modalTitle} />
			</Row>
		);
	}
}

export default ElementCategoryList;
