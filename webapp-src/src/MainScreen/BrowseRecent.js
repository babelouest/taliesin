import React, { Component } from 'react';
import { Row, Col, ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ElementPathIcon from './ElementPathIcon';
import ElementPathList from './ElementPathList';
import ElementCategoryIcon from './ElementCategoryIcon';
import ElementCategoryList from './ElementCategoryList';
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
                                        case 2: // Artist
						var list = [];
						result.forEach((element) => {
                                                	if ((element.type === "audio" || element.type === "video") && element.tags && element.tags.artist) {
								if (!list.find((elt) => {
									return elt.tags.artist === element.tags.artist;
								})) {
									var newElt = {data_source: element.data_source, path: element.path, name: element.name, tags: element.tags};
		   	 						list.push(newElt);
								}
                                                        }
						});
						this.setState({loaded: true, elementListInitial: list, elementList: list});
                                                break;
                                        case 3: // Album
						var list = [];
						result.forEach((element) => {
                                                	if ((element.type === "audio" || element.type === "video") && element.tags && element.tags.album) {
								if (!list.find((elt) => {
									return elt.tags.album === element.tags.album;
								})) {
									var newElt = {data_source: element.data_source, path: element.path, name: element.name, tags: element.tags};
		   	 						list.push(newElt);
								}
                                                        }
						});
						this.setState({loaded: true, elementListInitial: list, elementList: list});
                                                break;
                                        case 4: // Date
						var list = [];
						result.forEach((element) => {
                                                	if ((element.type === "audio" || element.type === "video") && element.tags && element.tags.date) {
								if (!list.find((elt) => {
									return elt.tags.date === element.tags.date;
								})) {
									var newElt = {data_source: element.data_source, path: element.path, name: element.name, tags: element.tags};
		   	 						list.push(newElt);
								}
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
					<span className="hidden-xs space-after">{i18n.t("common.group_by")}</span>
                                        <ButtonGroup className="hidden-xs">
                                        	<Button onClick={() => this.handleChangeGroupBy(0)} title={i18n.t("common.file")} bsStyle={this.state.groupBy===0?"primary":"default"}>
                                                	{i18n.t("common.file")}
                                                </Button>
                                        	<Button onClick={() => this.handleChangeGroupBy(1)} title={i18n.t("common.folder")} bsStyle={this.state.groupBy===1?"primary":"default"}>
                                                	{i18n.t("common.folder")}
                                                </Button>
                                        	<Button onClick={() => this.handleChangeGroupBy(2)} title={i18n.t("common.artist")} bsStyle={this.state.groupBy===2?"primary":"default"}>
                                                	{i18n.t("common.artist")}
                                                </Button>
                                        	<Button onClick={() => this.handleChangeGroupBy(3)} title={i18n.t("common.album")} bsStyle={this.state.groupBy===3?"primary":"default"}>
                                                	{i18n.t("common.album")}
                                                </Button>
                                        	<Button onClick={() => this.handleChangeGroupBy(4)} title={i18n.t("common.date")} bsStyle={this.state.groupBy===4?"primary":"default"}>
                                                	{i18n.t("common.date")}
                                                </Button>
                                        </ButtonGroup>
					<DropdownButton className="visible-xs" id="grouypBy" title={
						<span>{i18n.t("common.group_by")}</span>
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
                                	if (this.state.groupBy === 0 || this.state.groupBy === 1) {
						currentList.push(
							<ElementPathIcon key={index} dataSource={element.data_source} path={element.path||"/"} element={element} />
						);
                                        } else if (this.state.groupBy === 2) {
                                                var artistElt = {name: element.tags.artist, type: "artist"};
                                        	currentList.push(
                                                	<ElementCategoryIcon key={index} dataSource={element.dataSource} category="artist" categoryValue={element.tags.artist} element={artistElt} />
                                                );
                                        } else if (this.state.groupBy === 3) {
                                                var artistElt = {name: element.tags.album, type: "album"};
                                        	currentList.push(
                                                	<ElementCategoryIcon key={index} dataSource={element.dataSource} category="album" categoryValue={element.tags.album} element={artistElt} />
                                                );
                                        } else if (this.state.groupBy === 4) {
                                                var artistElt = {name: element.tags.date, type: "year"};
                                        	currentList.push(
                                                	<ElementCategoryIcon key={index} dataSource={element.dataSource} category="year" categoryValue={element.tags.date} element={artistElt} />
                                                );
                                        }
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
                                	if (this.state.groupBy === 0 || this.state.groupBy === 1) {
						currentList.push(
							<ElementPathList key={index} dataSource={element.data_source} path={element.path||"/"} element={element} />
						);
                                        } else if (this.state.groupBy === 2) {
                                                var artistElt = {name: element.tags.artist, type: "artist"};
                                        	currentList.push(
                                                	<ElementCategoryList key={index} dataSource={element.dataSource} category="artist" categoryValue={element.tags.artist} subCategory={false} subCategoryValue={false} element={artistElt} />
                                                );
                                        } else if (this.state.groupBy === 3) {
                                                var artistElt = {name: element.tags.album, type: "album"};
                                        	currentList.push(
                                                	<ElementCategoryList key={index} dataSource={element.dataSource} category="album" categoryValue={element.tags.album} subCategory={false} subCategoryValue={false} element={artistElt} />
                                                );
                                        } else if (this.state.groupBy === 4) {
                                                var artistElt = {name: element.tags.date, type: "year"};
                                        	currentList.push(
                                                	<ElementCategoryList key={index} dataSource={element.dataSource} category="year" categoryValue={element.tags.date} subCategory={false} subCategoryValue={false} element={artistElt} />
                                                );
                                        }
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
