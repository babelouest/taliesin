import React, { Component } from 'react';
import { Breadcrumb } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class BreadCrumbMenuCategory extends Component {
	constructor(props) {
		super(props);
		this.state = {
			dataSource: props.dataSource, 
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue,
		};
		
		this.handleGotoCategory = this.handleGotoCategory.bind(this);
		this.handleGotoCategoryValue = this.handleGotoCategoryValue.bind(this);
		this.handleGotoSubcategory = this.handleGotoSubcategory.bind(this);
		this.handleGotoSubcategoryValue = this.handleGotoSubcategoryValue.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSource: nextProps.dataSource, 
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue
		})
	}
	
	handleGotoCategory() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentCategory", category: this.state.category});
	}
	
	handleGotoCategoryValue() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue});
	}
	
	handleGotoSubcategory() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue, subCategory: this.state.subCategory});
	}
	
	handleGotoSubcategoryValue() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue, subCategory: this.state.subCategory, subCategoryValue: this.state.subCategoryValue});
	}
	
	render() {
		var breadcrumbList = [
			<Breadcrumb.Item key={0} onClick={this.handleGotoCategory}>
				{this.state.category}
			</Breadcrumb.Item>
		];
		if (this.state.categoryValue) {
			breadcrumbList.push(
				<Breadcrumb.Item key={1} onClick={this.handleGotoCategoryValue}>
					{this.state.categoryValue}
				</Breadcrumb.Item>
			)
			if (this.state.subCategory) {
				breadcrumbList.push(
					<Breadcrumb.Item key={2} onClick={this.handleGotoSubcategory}>
						{this.state.subCategory}
					</Breadcrumb.Item>
				)
				if (this.state.subCategoryValue) {
					breadcrumbList.push(
						<Breadcrumb.Item key={3} onClick={this.handleGotoSubcategoryValue}>
							{this.state.subCategoryValue}
						</Breadcrumb.Item>
					)
				}
			}
		}
		return (
			<Breadcrumb>
				<Breadcrumb.Item>
					{this.state.dataSource}
				</Breadcrumb.Item>
				{breadcrumbList}
			</Breadcrumb>
		);
	}
}

export default BreadCrumbMenuCategory;
