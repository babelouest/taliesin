import React, { Component } from 'react';
import { Row, Col, ButtonGroup, Button } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ElementPathIcon from './ElementPathIcon';
import ElementPathList from './ElementPathList';
import i18n from '../lib/i18n';

class BrowsePath extends Component {
	constructor(props) {
		super(props);
		this.state = {
			dataSource: props.dataSource, 
			path: props.path, 
			view: props.view, 
			elementList: [], 
			elementListInitial: [], 
			filter: "", 
			loaded: false,
			offset: 0,
			limit: 100
		};
		
		this.getElementList();
		
		this.filterList = this.filterList.bind(this);
		this.navigate = this.navigate.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSource: nextProps.dataSource, 
			path: nextProps.path, 
			view: nextProps.view, 
			elementList: [], 
			elementListInitial: [], 
			filter: "", 
			loaded: false,
			offset: 0,
			limit: 100
		}, () => {
			this.getElementList();
		});
	}
	
	filterList(event) {
		var newList = this.state.elementListInitial;
		newList = newList.filter((item) => {
			return item.name.toLowerCase().search(event.target.value.toLowerCase()) !== -1;
		});
		this.setState({elementList: newList, offset: 0});
	}
	
	getElementList() {
		if (!this.state.loaded && this.state.dataSource) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23").replace(/\+/g, "%2B"))
			.then((result) => {
				this.setState({loaded: true, elementListInitial: result, elementList: result});
			})
			.fail((result) => {
				this.setState({loaded: true, elementListInitial: [], elementList: []});
			});
		}
	}
	
	navigate(o) {
		this.setState({offset: this.state.offset+o});
	}
	
	render() {
		var currentList = [];
		var currentElementList = this.state.elementList, index;
		
		if (this.state.loaded) {
			if (this.state.view === "icon") {
				for (index in currentElementList) {
					if (index >= this.state.offset) {
						currentList.push(
							<ElementPathIcon key={index} dataSource={this.state.dataSource} path={this.state.path} element={currentElementList[index]} />
						);
					}
					if (index > this.state.offset + this.state.limit) {
						break;
					}
				}
				
				return (
					<div>
						<Row>
							<input type="text" placeholder={i18n.t("common.filter")} value={this.filter} className="form-control" onChange={this.filterList}/>
						</Row>
						<Row>
							{currentList}
						</Row>
					</div>
				);
			} else {
				for (index in currentElementList) {
					if (index >= this.state.offset) {
						currentList.push(
							<ElementPathList key={index} dataSource={this.state.dataSource} path={this.state.path} element={currentElementList[index]} />
						);
					}
					if (index > this.state.offset + this.state.limit) {
						break;
					}
				}
				
				return (
					<div>
						<Row style={{marginTop: "10px"}}>
							<Col md={12} sm={12} xs={12} className="text-right">
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
						<Row>
							<input type="text" placeholder="Filter" value={this.filter} className="form-control" onChange={this.filterList}/>
						</Row>
						{currentList}
						<Row style={{marginTop: "10px"}}>
							<Col md={12} sm={12} xs={12} className="text-right">
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
			}
		} else {
			return (
				<Row>
					<FontAwesome name="spinner" size="2x" spin />
				</Row>
			);
		}
	}
}

export default BrowsePath;
