import React, { Component } from 'react';
import { Row, Col, ButtonToolbar, ToggleButton, ToggleButtonGroup, ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ElementPathIcon from './ElementPathIcon';
import ElementPathList from './ElementPathList';
import i18n from '../lib/i18n';

class BrowseRecent extends Component {
	constructor(props) {
		super(props);
		this.state = {view: props.view, elementList: [], elementListInitial: [], filter: "", loaded: false, groupBy: 1, offset: 0};
		
		this.getElementList();
		
		this.filterList = this.filterList.bind(this);
		this.handleChangeGroupBy = this.handleChangeGroupBy.bind(this);
		this.handleNavigationPrevious = this.handleNavigationPrevious.bind(this);
		this.handleNavigationNext = this.handleNavigationNext.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({view: nextProps.view, elementList: [], elementListInitial: [], filter: "", loaded: false}, () => {
			this.getElementList();
		});
	}
	
	filterList(event) {
		var newList = this.state.elementListInitial;
		newList = newList.filter((item) => {
			return item.name.toLowerCase().search(event.target.value.toLowerCase()) !== -1;
		});
		this.setState({elementList: newList});
	}
	
	handleNavigationNext() {
		this.setState({offset: this.state.offset + 100, loaded: false}, () => {
			this.getElementList();
		});
	}
	
	handleNavigationPrevious() {
		this.setState({offset: this.state.offset - 100, loaded: false}, () => {
			this.getElementList();
		});
	}
	
	getElementList() {
		if (!this.state.loaded) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/search/", {sort: "last_updated", sort_direction: "desc", offset: this.state.offset})
			.then((result) => {
				switch (this.state.groupBy) {
					case 0: // File
						this.setState({loaded: true, elementListInitial: result, elementList: result});
						break;
					case 1: // Folder
						var list = [];
						result.forEach((element) => {
							var pathParent = element.path.substring(0, element.path.lastIndexOf("/"));
							var path = pathParent.substring(0, pathParent.lastIndexOf("/"));
							var name = pathParent.substring(pathParent.lastIndexOf("/") + 1);
							if (!list.find((elt) => {
								return elt.pathParent === pathParent && elt.data_source === element.data_source;
							})) {
								var newElt = {data_source: element.data_source, type: "folder", path: path, name: name, pathParent: pathParent};
								list.push(newElt);
							}
						});
						this.setState({loaded: true, elementListInitial: list, elementList: list});
						break;
					default: // Nope
						this.setState({loaded: true, elementListInitial: [], elementList: []});
						break;
				}
			})
			.fail((result) => {
				this.setState({loaded: true, elementListInitial: [], elementList: []});
			});
		}
	}
	
	handleChangeGroupBy(e) {
		this.setState({groupBy: e, loaded: false}, () => {
			this.getElementList();
		});
	}
	
	render() {
		var currentList = [];
		var currentElementList = this.state.elementList;
		var header =
			<Row>
				<Col md={6} sm={6} xs={6}>
					<span className="hidden-xs">Group by</span>
					<ButtonToolbar className="hidden-xs">
						<ToggleButtonGroup type="radio" name="groupBy" value={this.state.groupBy} onChange={this.handleChangeGroupBy}>
							<ToggleButton value={0}>{i18n.t("common.file")}</ToggleButton>
							<ToggleButton value={1}>{i18n.t("common.folder")}</ToggleButton>
							<ToggleButton value={2}>{i18n.t("common.artist")}</ToggleButton>
							<ToggleButton value={3}>{i18n.t("common.album")}</ToggleButton>
							<ToggleButton value={4}>{i18n.t("common.date")}</ToggleButton>
						</ToggleButtonGroup>
					</ButtonToolbar>
					<DropdownButton className="visible-xs" id="grouypBy" title={
						<span>Group by</span>
					}>
						<MenuItem onClick={() => this.handleChangeGroupBy(0)} className={this.state.groupBy===0?"bg-success":""}>
							{i18n.t("common.file")}
						</MenuItem>
						<MenuItem onClick={() => this.handleChangeGroupBy(1)} className={this.state.groupBy===1?"bg-success":""}>
							{i18n.t("common.folder")}
						</MenuItem>
						<MenuItem onClick={() => this.handleChangeGroupBy(2)} className={this.state.groupBy===2?"bg-success":""}>
							{i18n.t("common.artist")}
						</MenuItem>
						<MenuItem onClick={() => this.handleChangeGroupBy(3)} className={this.state.groupBy===3?"bg-success":""}>
							{i18n.t("common.album")}
						</MenuItem>
						<MenuItem onClick={() => this.handleChangeGroupBy(4)} className={this.state.groupBy===4?"bg-success":""}>
							{i18n.t("common.date")}
						</MenuItem>
					</DropdownButton>
				</Col>
				<Col md={6} sm={6} xs={6} className="text-right">
					<span className="hidden-xs">{i18n.t("common.navigation")} - </span><span className="space-after">{i18n.t("common.page", {page: ((this.state.offset/100)+1)})}</span><br className="hidden-xs"/>
					<ButtonGroup>
						<Button onClick={this.handleNavigationPrevious} disabled={!this.state.offset}>
							<span className="hidden-xs">{i18n.t("common.previous_page")}</span>
							<FontAwesome name="chevron-left" className="visible-xs" />
						</Button>
						<Button onClick={this.handleNavigationNext}>
							<span className="hidden-xs">{i18n.t("common.next_page")}</span>
							<FontAwesome name="chevron-right" className="visible-xs" />
						</Button>
					</ButtonGroup>
				</Col>
			</Row>;
		if (this.state.loaded) {
			if (this.state.view === "icon") {
				currentElementList.forEach((element, index) => {
					currentList.push(
						<ElementPathIcon key={index} dataSource={element.data_source} path={element.path} element={element} />
					);
				});
				
				return (
					<div>
						{header}
						<Row>
							<input type="text" placeholder="Filter" value={this.filter} className="form-control" onChange={this.filterList}/>
						</Row>
						<Row>
							{currentList}
						</Row>
					</div>
				);
			} else {
				currentElementList.forEach((element, index) => {
					currentList.push(
						<ElementPathList key={index} dataSource={element.data_source} path={element.path} element={element} />
					);
				});
				
				return (
					<div>
						{header}
						<Row>
							<input type="text" placeholder={i18n.t("common.filter")} value={this.filter} className="form-control" onChange={this.filterList}/>
						</Row>
						{currentList}
					</div>
				);
			}
		} else {
			return (
				<div>
					{header}
					<Row>
						<FontAwesome name="spinner" size="2x" spin />
					</Row>
				</div>
			);
		}
	}
}

export default BrowseRecent;
