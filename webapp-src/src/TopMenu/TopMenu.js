import React, { Component } from 'react';
import { Navbar, Nav, NavItem, FormGroup, FormControl, NavDropdown, MenuItem, InputGroup, Button, Popover, OverlayTrigger, Panel, ListGroup, ListGroupItem, Image, Row, Col } from 'react-bootstrap';
import LoginButton from './LoginButton';
import FontAwesome from 'react-fontawesome';
import DataSourceList from './DataSourceList';
import StateStore from '../lib/StateStore';
import ModalMedia from '../Modal/ModalMedia';
import i18n from '../lib/i18n';

class TopMenu extends Component {
  constructor(props) {
    super(props);
		
		this.uniquePopoverStore = {};
		this.uniqueKey = 1;
		
    this.state = {
      view: StateStore.getState().profile.view,
			browse: StateStore.getState().profile.browse,
      dataSourceList: StateStore.getState().dataSourceList,
      dataSource: StateStore.getState().profile.dataSource,
			searchPattern: "",
			searchTimeout: false,
			searching: true,
			searchResult: {},
			modalShow: false,
			modalMedia: false,
			modalTitle: ""
    };
		
		this.handleSelectCategory = this.handleSelectCategory.bind(this);
		this.handleBrowsePath = this.handleBrowsePath.bind(this);
		this.handleBrowseRecent = this.handleBrowseRecent.bind(this);
		this.handleSelectView = this.handleSelectView.bind(this);
		this.handleChangeSearchPattern = this.handleChangeSearchPattern.bind(this);
		this.handleAdvancedSearch = this.handleAdvancedSearch.bind(this);
		this.simpleSearch = this.simpleSearch.bind(this);
		this.runSimpleSearch = this.runSimpleSearch.bind(this);
		this.getSearchResultCovers = this.getSearchResultCovers.bind(this);
		this.openPlaylist = this.openPlaylist.bind(this);
		this.openStream = this.openStream.bind(this);
		this.openFolder = this.openFolder.bind(this);
		this.openMedia = this.openMedia.bind(this);
		this.closeMedia = this.closeMedia.bind(this);
		this.closeSearch = this.closeSearch.bind(this);
		this.handlechangeLanguage = this.handlechangeLanguage.bind(this);
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setCurrentDataSource") {
				this.setState({dataSource: reduxState.profile.dataSource});
			} else if (reduxState.lastAction === "setDataSource") {
				this.setState({dataSourceList: reduxState.dataSourceList, dataSource: reduxState.profile.dataSource});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			} else if (reduxState.lastAction === "setCurrentBrowse") {
				this.setState({browse: reduxState.profile.browse});
			}
		});
  }
	
	componentDidMount() {
		this.uniqueKey++;
		this.uniquePopoverStore[this.uniqueKey] = this.refs.myPopover;
	}

	componentWillUnmount() {
		delete this.uniquePopoverStore[this.uniqueKey];
	}
	
	handleSelectCategory(event) {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentCategory", category: event});
	}
  
	handleBrowsePath() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentPath", path: ""});
	}
	
	handleBrowseDashboard() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "dashboard"});
	}
	
	handleBrowseRecent() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "recent"});
	}
	
	handleBrowsePlylist() {
		StateStore.dispatch({type: "setCurrentPlaylist", playlist: false});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "playlist"});
	}
	
	handleAdvancedSearch() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "advancedSearch"});
	}
	
	handleSelectView(event) {
		StateStore.dispatch({type: "setCurrentView", view: event});
	}
	
	handleChangeSearchPattern(event) {
		this.refs.myPopover.hide();
		this.setState({searchPattern: event.target.value, searchResult: {}}, () => {
			this.simpleSearch();
		});
	}
	
	simpleSearch() {
		if (this.state.searchTimeout) {
			clearTimeout(this.state.searchTimeout);
		}
		var searchTimeout = setTimeout(this.runSimpleSearch, 1000);
		this.setState({searchTimeout: searchTimeout, searching: true});
	}
	
	runSimpleSearch() {
		this.refs.myPopover.show();
		if (this.state.searchPattern !== "" && !Object.keys(this.state.searchResult).length) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/search/?q=" + this.state.searchPattern)
			.then((result) => {
				this.setState({searchResult: result, searching: false}, () => {
					this.getSearchResultCovers();
				});
			})
			.fail(() => {
				this.refs.myPopover.hide();
			});
		}
	}
	
	closeSearch() {
		this.refs.myPopover.hide();
	}
	
	getSearchResultCovers() {
		var searchResult = this.state.searchResult;
		if (searchResult.playlist) {
			searchResult.playlist.forEach((pl, index) => {
				if (!pl.cover) {
					StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + pl.name + "?cover&thumbnail&base64")
					.then((cover) => {
						pl.cover = cover;
						this.setState({searchResult: searchResult});
					});
				}
			});
		}
		if (searchResult.folder) {
			searchResult.folder.forEach((folder) => {
				if (!folder.cover) {
					StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(folder.data_source) + "/browse/path/" + encodeURI(folder.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
					.then((cover) => {
						folder.cover = cover;
						this.setState({searchResult: searchResult});
					});
				}
			});
		}
		if (searchResult.media) {
			searchResult.media.forEach((media) => {
				if (!media.cover) {
					StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(media.data_source) + "/browse/path/" + encodeURI(media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
					.then((cover) => {
						media.cover = cover;
						this.setState({searchResult: searchResult});
					});
				}
			});
		}
	}
	
	openPlaylist(playlist) {
		this.refs.myPopover.hide();
		StateStore.dispatch({type: "setCurrentPlaylist", playlist: playlist});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "playlist"});
	}
	
	openStream(stream) {
		this.refs.myPopover.hide();
		StateStore.dispatch({type: "setStreamDetails", stream: stream});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "streamDetails"});
	}
	
	openFolder(folder) {
		this.refs.myPopover.hide();
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === folder.data_source})});
		StateStore.dispatch({type: "setCurrentPath", path: folder.path });
	}
	
	openMedia(media) {
		this.refs.myPopover.hide();
		this.setState({modalShow: true, modalMedia: media, modalTitle: (media.tags.title||media.name)});
	}
	
	closeMedia() {
		this.setState({modalShow: false});
	}
  
  handlechangeLanguage(lang) {
    i18n.changeLanguage(lang);
  }
	
	render() {
		if (StateStore.getState().status === "connected") {
			var searchOverlay = 
				<Popover id="searchResult">
						{i18n.t("topmenu.searching")} <FontAwesome name="spinner" spin />
				</Popover>;
			if (!this.state.searching) {
				var playlistResult, streamResult, folderResult, mediaResult;
				if (this.state.searchResult.playlist) {
					var playlistList = [];
					this.state.searchResult.playlist.forEach((pl, index) => {
						if (index < 10) {
							var cover;
							if (pl.cover) {
								cover = <Image src={"data:image/jpeg;base64," + pl.cover} responsive style={{maxWidth: "40px", maxHeight: "40px"}}/>
							}
							playlistList.push(
								<ListGroupItem key={index} onClick={() => {this.openPlaylist(pl)}}>
									<Row>
										<Col xs={4}>
											{cover}
										</Col>
										<Col xs={8}>
											{pl.name}
										</Col>
									</Row>
								</ListGroupItem>
							);
						}
					});
					playlistResult = 
					<Panel collapsible defaultExpanded header={i18n.t("topmenu.search_result_playlist")}>
						<ListGroup fill>
							{playlistList}
						</ListGroup>
					</Panel>
				}
				if (this.state.searchResult.stream) {
					var streamList = [];
					this.state.searchResult.stream.forEach((str, index) => {
						if (index < 10) {
							streamList.push(
								<ListGroupItem key={index} onClick={() => {this.openStream(str)}}>
									<Row>
										<Col xs={12}>
											{str.display_name}
										</Col>
									</Row>
								</ListGroupItem>
							);
						}
					});
					streamResult = 
					<Panel collapsible defaultExpanded header={i18n.t("topmenu.search_result_stream")}>
						<ListGroup fill>
							{streamList}
						</ListGroup>
					</Panel>
				}
				if (this.state.searchResult.folder) {
					var folderList = [];
					this.state.searchResult.folder.forEach((folder, index) => {
						if (index < 10) {
							var cover;
							if (folder.cover) {
								cover = <Image src={"data:image/jpeg;base64," + folder.cover} responsive style={{maxWidth: "40px", maxHeight: "40px"}}/>
							}
							folderList.push(
								<ListGroupItem key={index} onClick={() => {this.openFolder(folder)}}>
									<Row>
										<Col xs={4}>
											{cover}
										</Col>
										<Col xs={8}>
											{folder.name}
										</Col>
									</Row>
								</ListGroupItem>
							);
						}
					});
					folderResult = 
					<Panel collapsible defaultExpanded header={i18n.t("topmenu.search_result_folder")}>
						<ListGroup fill>
							{folderList}
						</ListGroup>
					</Panel>
				}
				if (this.state.searchResult.media) {
					var mediaList = [];
					this.state.searchResult.media.forEach((media, index) => {
						if (index < 10) {
							var title = media.name;
							if (media.tags) {
								if (media.tags.artist) {
									title = media.tags.artist + " - ";
								} else if (media.tags.album_artist) {
									title = media.tags.album_artist + " - ";
								} else {
									title = "";
								}
								if (media.tags.title) {
									title += media.tags.title;
								} else {
									title += media.name;
								}
							}
							var cover;
							if (media.cover) {
								cover = <Image src={"data:image/jpeg;base64," + media.cover} responsive style={{maxWidth: "40px", maxHeight: "40px"}}/>
							}
							mediaList.push(
								<ListGroupItem key={index} onClick={() => {this.openMedia(media)}}>
									<Row>
										<Col xs={4}>
											{cover}
										</Col>
										<Col xs={8}>
											{title}
										</Col>
									</Row>
								</ListGroupItem>
							);
						}
					});
					mediaResult = 
					<Panel collapsible defaultExpanded header={i18n.t("topmenu.search_result_media")}>
						<ListGroup fill>
							{mediaList}
						</ListGroup>
					</Panel>
				}
				searchOverlay =
					<Popover id="searchResult">
							{i18n.t("topmenu.search_result")} <strong>{this.state.searchPattern}</strong>
							&nbsp;<Button title={i18n.t("topmenu.close")} onClick={this.closeSearch}>
								<FontAwesome name={"close"} />
							</Button>
							{playlistResult}
							{streamResult}
							{folderResult}
							{mediaResult}
					</Popover>;
			}
      var languages = [];
      ["en","fr"].forEach((lang, index) => {
        if (lang === i18n.language) {
          languages.push(<MenuItem key={index} className="bg-success">{lang}</MenuItem>);
        } else {
          languages.push(<MenuItem key={index} onClick={() => {this.handlechangeLanguage(lang)}}>{lang}</MenuItem>);
        }
      });
			return (
				<div>
					<Navbar collapseOnSelect>
						<Navbar.Header>
							<Navbar.Brand>
								Taliesin
							</Navbar.Brand>
							<Navbar.Toggle />
						</Navbar.Header>
						<Navbar.Collapse>
							<Nav>
								<NavDropdown title={i18n.t("topmenu.browse")} id="nav-categories">
									<MenuItem onClick={() => this.handleBrowseDashboard()} className={this.state.browse==="dashboard"?"bg-success":""}>{i18n.t("topmenu.dashboard")}</MenuItem>
									<MenuItem onClick={() => this.handleBrowsePath()} className={this.state.browse==="file"?"bg-success":""}>{i18n.t("topmenu.files")}</MenuItem>
									<MenuItem onClick={() => this.handleBrowsePlylist()} className={this.state.browse==="playlist"?"bg-success":""}>{i18n.t("topmenu.playlists")}</MenuItem>
									<MenuItem onClick={() => this.handleBrowseRecent()} className={this.state.browse==="recent"?"bg-success":""}>{i18n.t("topmenu.recent")}</MenuItem>
									<MenuItem divider />
									<MenuItem onClick={() => this.handleSelectCategory("artist")}>{i18n.t("topmenu.artists")}</MenuItem>
									<MenuItem onClick={() => this.handleSelectCategory("album")}>{i18n.t("topmenu.albums")}</MenuItem>
									<MenuItem onClick={() => this.handleSelectCategory("year")}>{i18n.t("topmenu.years")}</MenuItem>
									<MenuItem onClick={() => this.handleSelectCategory("genre")}>{i18n.t("topmenu.genres")}</MenuItem>
								</NavDropdown>
								<DataSourceList list={this.state.dataSourceList} dataSource={this.state.dataSource}/>
								<NavDropdown title={i18n.t("topmenu.view")} id="nav-view">
									<MenuItem onClick={() => {this.handleSelectView("list")}} className={this.state.view==="list"?"bg-success":""}>{i18n.t("topmenu.list")}</MenuItem>
									<MenuItem onClick={() => {this.handleSelectView("icon")}} className={this.state.view==="icon"?"bg-success":""}>{i18n.t("topmenu.icon")}</MenuItem>
								</NavDropdown>
								<NavItem onClick={() => this.handleAdvancedSearch()}>{i18n.t("topmenu.advanced_search")}</NavItem>
							</Nav>
							<Nav pullRight>
								<NavDropdown title={i18n.t("topmenu.lang")} id="nav-view">
                  {languages}
								</NavDropdown>
								<LoginButton></LoginButton>
							</Nav>
							<Navbar.Form>
								<FormGroup>
									<OverlayTrigger ref='myPopover' container={document.body} trigger={null} placement="bottom" overlay={searchOverlay}>
										<InputGroup>
											<FormControl type="text" placeholder={i18n.t("topmenu.search")} value={this.state.searchPattern} onChange={this.handleChangeSearchPattern} />
											<InputGroup.Button>
												<Button onClick={this.runSimpleSearch}>
													<FontAwesome name={"search"} />
												</Button>
											</InputGroup.Button>
										</InputGroup>
									</OverlayTrigger>
								</FormGroup>
							</Navbar.Form>
						</Navbar.Collapse>
					</Navbar>
					<ModalMedia show={this.state.modalShow} media={this.state.modalMedia} title={this.state.modalTitle} onClose={this.closeMedia} />
				</div>
			);
		} else {
			return (
				<Navbar collapseOnSelect>
					<Navbar.Header>
						<Navbar.Brand>
							Taliesin
						</Navbar.Brand>
					</Navbar.Header>
					<Navbar.Collapse>
						<Nav pullRight>
							<LoginButton></LoginButton>
						</Nav>
					</Navbar.Collapse>
				</Navbar>
			);
		}
	}
}

export default TopMenu;
