import React, { Component } from 'react';
import { Row, Col, ButtonGroup, Button } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import ElementCategoryList from './ElementCategoryList';
import ElementCategoryIcon from './ElementCategoryIcon';
import CategoryButtons from './CategoryButtons';
import i18n from '../lib/i18n';

class BrowseCategory extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			dataSource: props.dataSource, 
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue, 
			currentCategory: props.subCategory||props.category,
			currentValue: props.subCategoryValue||props.categoryValue,
			offset: 0,
			limit: 100,
			view: props.view, 
			elementList: [], 
			elementListInitial: [], 
			filter: "", 
			loaded: false
		};
		
		this.getElementList();
		
		this.filterList = this.filterList.bind(this);
		this.navigate = this.navigate.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSource: nextProps.dataSource, 
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue, 
			currentCategory: nextProps.subCategory||nextProps.category,
			currentValue: nextProps.subCategoryValue||nextProps.categoryValue,
			offset: 0,
			limit: 100,
			view: nextProps.view, 
			elementList: [], 
			elementListInitial: [], 
			filter: "", 
			loaded: false
		}, () => {
			this.getElementList();
		});
	}
	
	getElementList() {
		if (!this.state.loaded && this.state.dataSource) {
			var url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/category/" + encodeURI(this.state.category);
			if (this.state.categoryValue) {
				url += "/" + encodeURIComponent(this.state.categoryValue);
				if (this.state.subCategory) {
					url += "/" + this.state.subCategory;
					if (this.state.subCategoryValue) {
						url += "/" + encodeURIComponent(this.state.subCategoryValue);
					}
				}
			}
			StateStore.getState().APIManager.taliesinApiRequest("GET", url)
			.then((result) => {
				this.setState({loaded: true, elementListInitial: result, elementList: result});
			})
			.fail((result) => {
				this.setState({loaded: true, elementListInitial: [], elementList: []});
			});
		}
	}

	filterList(event) {
		var newList = this.state.elementListInitial;
		newList = newList.filter((item) => {
			return item.name.toLowerCase().search(event.target.value.toLowerCase()) !== -1;
		});
		this.setState({elementList: newList});
	}
	
	navigate(o) {
		this.setState({offset: this.state.offset+o});
	}
	
	render() {
		var currentList = [];
		var currentElementList = this.state.elementList;
		var index;
		
		if (this.state.loaded) {
			for (index in currentElementList) {
				if (index >= this.state.offset) {
					var categoryValue = this.state.categoryValue?this.state.categoryValue:currentElementList[index].name;
					var subCategoryValue = this.state.categoryValue?currentElementList[index].name:false;
					if (this.state.view === "icon") {
						currentList.push(
							<ElementCategoryIcon key={index} dataSource={this.state.dataSource} category={this.state.category} categoryValue={categoryValue} subCategory={this.state.subCategory} subCategoryValue={subCategoryValue} element={currentElementList[index]} />
						);
					} else {
						currentList.push(
							<ElementCategoryList key={index} dataSource={this.state.dataSource} category={this.state.category} categoryValue={categoryValue} subCategory={this.state.subCategory} subCategoryValue={subCategoryValue} element={currentElementList[index]} />
						);
					}
				}
				if (index > this.state.offset + this.state.limit) {
					break;
				}
			}
			return (
				<div>
					<Row style={{marginBottom: "10px"}}>
						<Col md={6} sm={8} xs={8}>
							<CategoryButtons category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue}/>
						</Col>
						<Col md={6} sm={4} xs={4} className="text-right">
							<ButtonGroup>
								<Button disabled={!this.state.offset} onClick={() => {this.navigate(-100)}}>
									<span className="hidden-sm hidden-xs">{i18n.t("common.previous_page")}</span>
									<span className="visible-sm visible-xs">
										<FontAwesome name="chevron-left" />
									</span>
								</Button>
								<Button disabled={(this.state.offset + this.state.limit) >= currentElementList.length} onClick={() => {this.navigate(100)}}>
									<span className="hidden-sm hidden-xs">{i18n.t("common.next_page")}</span>
									<span className="visible-sm visible-xs">
										<FontAwesome name="chevron-right" />
									</span>
								</Button>
							</ButtonGroup>
						</Col>
					</Row>
					<Row style={{marginBottom: "10px"}}>
						<input type="text" placeholder={i18n.t("common.filter")} value={this.filter} className="form-control" onChange={this.filterList}/>
					</Row>
					<Row>
					{currentList}
					</Row>
					<Row style={{marginTop: "10px"}}>
						<Col md={6} sm={8} xs={8}>
							<CategoryButtons category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue}/>
						</Col>
						<Col md={6} sm={4} xs={4} className="text-right">
							<ButtonGroup>
								<Button disabled={!this.state.offset} onClick={() => {this.navigate(-100)}}>
									<span className="hidden-sm hidden-xs">{i18n.t("common.previous_page")}</span>
									<span className="visible-sm visible-xs">
										<FontAwesome name="chevron-left" />
									</span>
								</Button>
								<Button disabled={(this.state.offset + this.state.limit) >= currentElementList.length} onClick={() => {this.navigate(100)}}>
									<span className="hidden-sm hidden-xs">{i18n.t("common.next_page")}</span>
									<span className="visible-sm visible-xs">
										<FontAwesome name="chevron-right" />
									</span>
								</Button>
							</ButtonGroup>
						</Col>
					</Row>
				</div>
			);
		} else {
			return (
				<Row>
					<FontAwesome name="spinner" size="2x" spin />
				</Row>
			);
		}
	}
}

export default BrowseCategory;
