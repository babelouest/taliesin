import React, { Component } from 'react';
import { Row, Col, Image, ButtonGroup, Button, MenuItem, DropdownButton } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class FullScreen extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			stream: StateStore.getState().profile.stream,
			media: StateStore.getState().profile.mediaNow,
			jukeboxIndex: StateStore.getState().profile.jukeboxIndex,
			currentPlayer: StateStore.getState().profile.currentPlayer,
			show: StateStore.getState().showFullScreen,
			title: "",
			imgBlob: false,
			status: StateStore.getState().profile.currentPlayerStatus,
			repeat: StateStore.getState().profile.currentPlayerRepeat,
			random: StateStore.getState().profile.currentPlayerRandom,
			volume: StateStore.getState().profile.currentPlayerVolume
		};
		
		StateStore.subscribe(() => {
			if (this._ismounted) {
				if (StateStore.getState().lastAction === "loadStream") {
					this.setState({stream: StateStore.getState().profile.stream}, () => {this.loadMedia();});
				} else if (StateStore.getState().lastAction === "setJukeboxIndex") {
					this.setState({jukeboxIndex: StateStore.getState().profile.jukeboxIndex}, () => {this.loadMedia();});
				} else if (StateStore.getState().lastAction === "setMediaNow") {
					this.setState({media: StateStore.getState().profile.mediaNow}, () => {this.loadMedia();});
				} else if (StateStore.getState().lastAction === "setCurrentPlayerStatus") {
					this.setState({
						status: StateStore.getState().profile.currentPlayerStatus,
						repeat: StateStore.getState().profile.currentPlayerRepeat,
						random: StateStore.getState().profile.currentPlayerRandom,
						volume: StateStore.getState().profile.currentPlayerVolume
					});
				} else if (StateStore.getState().lastAction === "showFullScreen") {
					this.setState({
						stream: StateStore.getState().profile.stream,
						media: StateStore.getState().profile.mediaNow,
						jukeboxIndex: StateStore.getState().profile.jukeboxIndex,
						show: StateStore.getState().showFullScreen,
						title: "",
						imgBlob: false,
						status: StateStore.getState().profile.currentPlayerStatus,
						repeat: StateStore.getState().profile.currentPlayerRepeat,
						random: StateStore.getState().profile.currentPlayerRandom,
						volume: StateStore.getState().profile.currentPlayerVolume
					}, () => {
						this.loadMedia();
						this.getMediaFolder();
					});
				} else if (StateStore.getState().lastAction === "loadStream" || StateStore.getState().lastAction === "loadStreamAndPlay") {
					this.setState({stream: StateStore.getState().profile.stream}, () => {this.loadMedia();});
				} else if (StateStore.getState().lastAction === "setCurrentPlayerStatus") {
					this.setState({
						status: StateStore.getState().profile.currentPlayerStatus,
						repeat: StateStore.getState().profile.currentPlayerRepeat,
						random: StateStore.getState().profile.currentPlayerRandom,
						volume: StateStore.getState().profile.currentPlayerVolume
					});
				}
			}
		});
		
		this.loadMedia = this.loadMedia.bind(this);
		this.buildTitle = this.buildTitle.bind(this);
		this.handleClose = this.handleClose.bind(this);
		this.getMediaCover = this.getMediaCover.bind(this);
		this.handleSelectArtist = this.handleSelectArtist.bind(this);
		this.handleSelectAlbum = this.handleSelectAlbum.bind(this);
		this.handleSelectYear = this.handleSelectYear.bind(this);
		this.handleSelectGenre = this.handleSelectGenre.bind(this);
		this.handlePlayerAction = this.handlePlayerAction.bind(this);
		this.handleChangeVolume = this.handleChangeVolume.bind(this);
		this.handleSelectFolder = this.handleSelectFolder.bind(this);
		this.getMediaFolder = this.getMediaFolder.bind(this);
		
		this.loadMedia();
		this.getMediaFolder();
	}

	componentWillReceiveProps(nextProps) {
		this.setState({
			stream: StateStore.getState().profile.stream,
			media: StateStore.getState().profile.mediaNow,
			jukeboxIndex: StateStore.getState().profile.jukeboxIndex,
			show: StateStore.getState().showFullScreen,
			title: "",
			imgBlob: false,
			status: StateStore.getState().profile.currentPlayerStatus,
			repeat: StateStore.getState().profile.currentPlayerRepeat,
			random: StateStore.getState().profile.currentPlayerRandom,
			volume: StateStore.getState().profile.currentPlayerVolume
		}, () => {
			this.loadMedia();
			this.getMediaFolder();
		});
	}
	
	componentWillUnmount() {
		this._ismounted = false;
	}
	
	componentDidMount() { 
		this._ismounted = true;
	}

	loadMedia() {
		if (this._ismounted) {
			this.setState({title: this.buildTitle()});
			this.getMediaCover();
		}
	}
	
	buildTitle() {
		var title = "";
		if (!!this.state.media) {
			if (this.state.jukeboxIndex > -1 && !this.state.stream.webradio) {
				title += ((this.state.jukeboxIndex+1)<10?"0"+(this.state.jukeboxIndex+1):(this.state.jukeboxIndex+1)) + "/" + (this.state.stream.elements<10?"0"+this.state.stream.elements:this.state.stream.elements) + " - ";
			}
			if (!!this.state.media.tags && !!this.state.media.tags.title) {
				title += this.state.media.tags.title;
			} else {
				title += this.state.media.name.replace(/\.[^/.]+$/, "");
			}
		}
		return title;
	}
	
	handleClose() {
		StateStore.dispatch({type: "showFullScreen", show: false});
	}
	
	handleSelectArtist(value) {
		StateStore.dispatch({type: "showFullScreen", show: false});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentCategory", category: "artist", categoryValue: value});
	}
	
	getMediaFolder() {
		var media = this.state.media;
		if (media) {
			if (media.path) {
				if (media.path.lastIndexOf("/") > -1) {
					media.folder = media.path.substring(0, media.path.lastIndexOf("/"));
				} else {
					media.folder = "";
				}
				this.setState({media: media});
			} else {
				media.folder = "";
				this.setState({media: media});
			}
		}
	}
	
	handleSelectAlbum(value) {
		StateStore.dispatch({type: "showFullScreen", show: false});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentCategory", category: "album", categoryValue: value});
	}
	
	handleSelectYear(value) {
		StateStore.dispatch({type: "showFullScreen", show: false});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentCategory", category: "year", categoryValue: value});
	}
	
	handleSelectGenre(value) {
		StateStore.dispatch({type: "showFullScreen", show: false});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentCategory", category: "genre", categoryValue: value});
	}
	
	handlePlayerAction(action) {
		StateStore.dispatch({type: "setPlayerAction", action: action});
	}
	
	handleChangeVolume(volume) {
		if (this._ismounted) {
			this.setState({volume: (this.state.volume+volume)}, () => {
				StateStore.dispatch({type: "setPlayerAction", action: "volume", parameter: (volume)});
			});
		}
	}
	
	handleSelectFolder(value) {
		StateStore.dispatch({type: "showFullScreen", show: false});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentPath", path: value});
	}
	
	getMediaCover() {
		if (!!this.state.media && this.state.show && this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&base64")
			.then((result) => {
				this.setState({imgBlob: result});
			})
			.fail(() => {
				this.setState({imgBlob: false});
			});
		}
	}
	
	render() {
		var mediaImage, metadata = [], separator, playButton;
		if (this.state.imgBlob) {
			mediaImage = <Image src={"data:image/jpeg;base64," + this.state.imgBlob} className="cover-image-full center-block" responsive />;
		} else {
			mediaImage = <Image src="images/generic-album.png" className="cover-image-full center-block" responsive />;
		}
		if (this.state.status === "stop") {
			playButton = 
				<Button title={i18n.t("common.play")} onClick={() => {this.handlePlayerAction("play")}}>
					<FontAwesome name={"play"} />
				</Button>;
		} else if (this.state.status === "pause") {
			playButton = 
				<Button title={i18n.t("common.play")} onClick={() => {this.handlePlayerAction("pause")}}>
					<FontAwesome name={"play"} />
				</Button>;
		} else if (!this.state.stream.webradio) {
			playButton = 
				<Button title={i18n.t("common.pause")} onClick={() => {this.handlePlayerAction("pause")}}>
					<FontAwesome name={"pause"} />
				</Button>;
		} else {
			playButton = 
				<Button title={i18n.t("common.play")} disabled={true}>
					<FontAwesome name={"play"} />
				</Button>;
		}
		if (this.state.media) {
			if (this.state.media.tags.title) {
				metadata.push(
					<Row key={0}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">{i18n.t("common.title")}</label>
						</Col>
						<Col xs={6}>
							<span className="text-fullscreen">{this.state.media.tags.title}</span>
						</Col>
					</Row>);
			}
			var dm = Math.floor(this.state.media.duration/60000);
			if (dm < 10) {
				dm = "0" + dm;
			}
			var ds = Math.floor(this.state.media.duration/1000)%60;
			if (ds < 10) {
				ds = "0" + ds;
			}
			metadata.push(
				<Row key={1}>
					<Col xs={6} className="text-right">
						<label className="text-fullscreen">{i18n.t("common.duration")}</label>
					</Col>
					<Col xs={6}>
						<span className="text-fullscreen">{dm}:{ds}</span>
					</Col>
				</Row>);
			if (this.state.media.tags.artist) {
				metadata.push(
					<Row key={2}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">{i18n.t("common.artist")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectArtist(this.state.media.tags.artist)}}>{this.state.media.tags.artist}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.album) {
				metadata.push(
					<Row key={3}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">{i18n.t("common.album")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectAlbum(this.state.media.tags.album)}}>{this.state.media.tags.album}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.date) {
				metadata.push(
					<Row key={4}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">{i18n.t("common.date")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectYear(this.state.media.tags.date)}}>{this.state.media.tags.date}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.genre) {
				metadata.push(
					<Row key={5}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">{i18n.t("common.genre")}</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectGenre(this.state.media.tags.genre)}}>{this.state.media.tags.genre}</a></span>
						</Col>
					</Row>);
			}
			metadata.push(
				<Row key={6}>
					<Col xs={6} className="text-right">
						<label className="text-fullscreen">{i18n.t("common.open_folder")}</label>
					</Col>
					<Col xs={6}>
						<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectFolder(this.state.media.folder)}}>{this.state.media.folder || "/"}</a></span>
					</Col>
				</Row>
			);
		}
		return (
			<div className={"fullscreen" + (!this.state.show?" hidden":"")} >
				<div className="media-background-fullscreen" style={{backgroundImage:this.state.imgBlob?"url(data:image/png;base64,"+this.state.imgBlob+")":"" }}>
				</div>
				<Row style={{marginTop: "10px"}}>
					<Col md={12} className="text-center">
						<div className="text-fullscreen">{this.state.title}</div>
					</Col>
				</Row>
				<Row style={{marginTop: "10px"}}>
					<Col md={12} className="text-center">
						<ButtonGroup>
							<Button title={i18n.t("common.previous")} onClick={() => {this.handlePlayerAction("previous")}}>
								<FontAwesome name={"fast-backward"} />
							</Button>
							<Button title={i18n.t("common.stop")} onClick={() => {this.handlePlayerAction("stop")}}>
								<FontAwesome name={"stop"} />
							</Button>
							{playButton}
							<Button title={i18n.t("common.next")} onClick={() => {this.handlePlayerAction("next")}}>
								<FontAwesome name={"fast-forward"} />
							</Button>
							<Button title={i18n.t("common.repeat")} onClick={() => {this.handlePlayerAction("repeat")}} className={(this.state.repeat&&!this.state.stream.webradio)?"btn-primary":""} disabled={this.state.stream.webradio}>
								<FontAwesome name={"repeat"} />
							</Button>
							<Button title={i18n.t("common.random")} onClick={() => {this.handlePlayerAction("random")}} className={(this.state.random&&!this.state.stream.webradio)?"btn-primary":""} disabled={this.state.stream.webradio}>
								<FontAwesome name={"random"} />
							</Button>
							<DropdownButton title={<FontAwesome name={"volume-up"} />} pullRight id="dropdown-volume">
								<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(5)}}>{i18n.t("common.volume_plus_5")}</MenuItem>
								<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(1)}}>{i18n.t("common.volume_plus_1")}</MenuItem>
								<MenuItem className="text-center">{i18n.t("common.volume_current")} {this.state.volume} %</MenuItem>
								<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-1)}}>{i18n.t("common.volume_minus_1")}</MenuItem>
								<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-5)}}>{i18n.t("common.volume_minus_5")}</MenuItem>
							</DropdownButton>
						</ButtonGroup>
					</Col>
				</Row>
				<Row style={{marginTop: "10px"}}>
					<Col md={12}>
						{mediaImage}
					</Col>
				</Row>
				{separator}
				{metadata}
				<Row style={{marginTop: "10px"}}>
					<Col md={12} className="text-center">
						<Button onClick={this.handleClose} className="btn btn-success">
							{i18n.t("common.close")}
						</Button>
					</Col>
				</Row>
			</div>
		);
	}
}

export default FullScreen;
