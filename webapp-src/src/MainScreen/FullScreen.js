import React, { Component } from 'react';
import { Row, Col, Image, ButtonGroup, Button, MenuItem, DropdownButton } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';

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
				} else if (StateStore.getState().lastAction === "showFullScreen") {
					this.setState({show: StateStore.getState().showFullScreen}, () => {this.loadMedia();});
				} else if (StateStore.getState().lastAction === "loadStream" || StateStore.getState().lastAction === "loadStreamAndPlay") {
					this.setState({stream: StateStore.getState().profile.stream}, () => {this.loadMedia();});
				}
			}
		});
		
		this.loadMedia();
		
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
			console.log("title", this.state.jukeboxIndex, this.state.stream);
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
	
	handleChangeVolume(event) {
		if (this._ismounted) {
			this.setState({volume: event.target.value}, () => {
				StateStore.dispatch({type: "setPlayerAction", action: "volume", parameter: (this.state.volume)});
			});
		}
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
		var mediaImage, metadata = [], separator, playButton, randomButton, repeatButton;
		if (this.state.imgBlob) {
			mediaImage = <Image src={"data:image/jpeg;base64," + this.state.imgBlob} className="cover-image-full center-block" responsive />;
		} else {
			mediaImage = <Image src="images/generic-album.png" className="cover-image-full center-block" responsive />;
		}
    if (this.state.play) {
      playButton = 
        <Button title="Play" onClick={() => {this.handlePlayerAction("pause")}}>
					<FontAwesome name={"pause"} />
        </Button>;
    } else {
      playButton = 
        <Button title="Play" onClick={() => {this.handlePlayerAction("play")}}>
					<FontAwesome name={"play"} />
        </Button>;
    }
		if (!this.state.stream.webradio) {
			randomButton =
				<Button title="Repeat list" onClick={() => {this.handlePlayerAction("repeat")}} disabled={this.state.stream.webradio} className={(this.state.repeat&&!this.state.stream.webradio)?"btn-primary":""}>
					<FontAwesome name={"repeat"} />
				</Button>;
			repeatButton =
				<Button title="Random" onClick={() => {this.handlePlayerAction("random")}} disabled={this.state.stream.webradio} className={(this.state.random&&!this.state.stream.webradio)?"btn-primary":""}>
					<FontAwesome name={"random"} />
				</Button>;
		}
		if (this.state.media) {
			if (this.state.media.tags.title) {
				metadata.push(
					<Row key={0}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">Title</label>
						</Col>
						<Col xs={6}>
							<span className="text-fullscreen">{this.state.media.tags.title}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.artist) {
				metadata.push(
					<Row key={1}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">Artist</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectArtist(this.state.media.tags.artist)}}>{this.state.media.tags.artist}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.album) {
				metadata.push(
					<Row key={2}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">Album</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectAlbum(this.state.media.tags.album)}}>{this.state.media.tags.album}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.date) {
				metadata.push(
					<Row key={3}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">Date</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectYear(this.state.media.tags.date)}}>{this.state.media.tags.date}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.genre) {
				metadata.push(
					<Row key={4}>
						<Col xs={6} className="text-right">
							<label className="text-fullscreen">Genre</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" className="anchor-fullscreen" onClick={() => {this.handleSelectGenre(this.state.media.tags.genre)}}>{this.state.media.tags.genre}</a></span>
						</Col>
					</Row>);
			}
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
              <Button title="Previous song" onClick={() => {this.handlePlayerAction("previous")}}>
                <FontAwesome name={"fast-backward"} />
              </Button>
              <Button title="Stop" onClick={() => {this.handlePlayerAction("stop")}}>
                <FontAwesome name={"stop"} />
              </Button>
              {playButton}
              <Button title="Next song" onClick={() => {this.handlePlayerAction("next")}}>
                <FontAwesome name={"fast-forward"} />
              </Button>
							{randomButton}
							{repeatButton}
              <DropdownButton title={<FontAwesome name={"volume-up"} />} id="dropdown-volume">
                <MenuItem eventKey="1"><input type="range" onChange={this.handleChangeVolume} value={this.state.volume} min="0" max="100" step="1"/></MenuItem>
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
              Close
            </Button>
					</Col>
				</Row>
			</div>
		);
	}
}

export default FullScreen;
