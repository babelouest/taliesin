import React, { Component } from 'react';
import { Row, Col, Image, Button, ButtonGroup, Label } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import VisibilitySensor from 'react-visibility-sensor';
import StateStore from '../lib/StateStore';
import ModalMedia from '../Modal/ModalMedia';
import i18n from '../lib/i18n';

class MediaRow extends Component {	
	constructor(props) {
		super(props);
		
		this.state = {
			stream: props.stream,
			media: props.media, 
			index: props.index,
			elements: props.elements,
			date: props.date,
			imgThumbBlob: false,
			coverLoaded: false,
			modalShow: false,
			modalTitle: this.buildTitle(props.media),
			visible: false,
			highlight: props.highlight
		};

		this.loadCover = this.loadCover.bind(this);
		this.handleOpenModal = this.handleOpenModal.bind(this);
		this.handlePlayNow = this.handlePlayNow.bind(this);
		this.handleRemove = this.handleRemove.bind(this);
		this.handleSelectDataSource = this.handleSelectDataSource.bind(this);
		this.handleSelectArtist = this.handleSelectArtist.bind(this);
		this.handleSelectAlbum = this.handleSelectAlbum.bind(this);
		this.onChangeVisibility = this.onChangeVisibility.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			stream: nextProps.stream,
			media: nextProps.media,
			index: nextProps.index,
			elements: nextProps.elements,
			date: nextProps.date,
			imgThumbBlob: false,
			coverLoaded: false,
			modalShow: false,
			modalTitle: this.buildTitle(nextProps.media),
			visible: false,
			highlight: nextProps.highlight
		});
	}
	
	componentDidMount() { 
		this._ismounted = true;
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	buildTitle(media, index, total) {
		var title = "";
		if (!!media) {
			if (index > -1) {
				title += ((index+1)<10?"0"+(index+1):(index+1)) + "/" + (total<10?"0"+total:total) + " - ";
			}
			if (!!media.tags) {
				if (index === -1) {
					if (media.tags.artist || media.tags.album_artist) {
						title += (media.tags.artist || media.tags.album_artist) + " - ";
					}
				}
				title += (media.tags.title || media.name.replace(/\.[^/.]+$/, ""));
			} else {
				title += media.name.replace(/\.[^/.]+$/, "");
			}
		}
		return title;
	}
	
	loadCover() {
		if (this.state.media.data_source && this.state.media.path && this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
			.then((result) => {
				this.setState({imgThumbBlob: result, coverLoaded: true});
			})
			.fail(() => {
				this.setState({imgThumbBlob: false, coverLoaded: true});
			});
		}
	}
	
	handleOpenModal() {
		if (this._ismounted) {
			this.setState({modalShow: true});
		}
	}
	
	handlePlayNow() {
		StateStore.dispatch({
			type: "loadStreamAndPlay", 
			stream: StateStore.getState().streamList.find((stream) => {
				return stream.name === this.state.stream
			}),
			index: this.state.index
		});
	}
	
	handleRemove() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream) + "/manage", {command: "remove_list", parameters: {index: this.state.index}})
		.then(() => {
			var list = StateStore.getState().streamList;
			for (var i in list) {
				if (list[i].name === this.state.stream) {
					list[i].elements--;
					StateStore.dispatch({type: "setStreamList", streamList: list});
					StateStore.dispatch({type: "setCurrentBrowse", browse: "showStreamMediaList"});
					break;
				}
			}
		});
	}
	
	handleSelectDataSource() {
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentPath", path: ""});
	}
	
	handleSelectArtist() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentCategory", category: "artist", categoryValue: (this.state.media.tags.artist || this.state.media.tags.album_artist)});
	}
	
	handleSelectAlbum() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
		StateStore.dispatch({type: "setCurrentCategory", category: "album", categoryValue: this.state.media.tags.album});
	}
	
	onChangeVisibility(isVisible) {
		this.setState({visible: isVisible}, () => {
			if (isVisible && !this.state.coverLoaded) {
				this.loadCover();	
			}
		});
	}
	
	render() {
		var artist, album, name, date, cover, firstCol;
		if (this.state.media.tags) {
			artist = this.state.media.tags.artist || this.state.media.tags.album_artist;
			album = this.state.media.tags.album;
			name = this.state.media.tags.title || this.state.media.name;
		} else {
			name = this.state.media.name;
		}
		if (this.state.date) {
			date = 
			<Col md={2} sm={12} xs={12}>
				<Label className="visible-xs visible-sm">Date</Label> {(new Date(this.state.date * 1000)).toLocaleString()}
			</Col>;
		}
		if (this.state.imgThumbBlob) {
			cover = <Image src={"data:image/jpeg;base64," + this.state.imgThumbBlob} thumbnail responsive className="cover-image-thumb"/>
		} else {
			cover = <Image src="/images/album-128.png" alt={this.state.media.name} className="cover-image-thumb" responsive />
		}
		if (this.state.stream && !this.state.stream.webradio) {
			firstCol =
				<Col md={2} sm={12} xs={12}>
					<ButtonGroup>
						<Button title={i18n.t("common.play_now")} onClick={this.handlePlayNow}>
							<FontAwesome name="play" />
						</Button>
						<Button title={i18n.t("common.remove_from_list")} onClick={this.handleRemove}>
							<FontAwesome name="trash" />
						</Button>
					</ButtonGroup>
					&nbsp;&nbsp;{(this.state.elements>=10&&this.state.index<9?"0":"") + (this.state.index + 1) + "/" + this.state.elements}
					&nbsp;{this.state.highlight?<FontAwesome name="music" />:""}
				</Col>;
		} else {
			firstCol =
				<Col md={2} sm={12} xs={12}>
					{this.state.highlight?<FontAwesome name="music" />:""}
				</Col>;
		}
		return (
			<div>
				<Row>
					<Col md={12}>
						<hr/>
					</Col>
				</Row>
				<Row className={ "row-media" + (this.state.highlight?" bg-success":"") }>
					{date}
					{firstCol}
					<Col md={2} sm={12} xs={12}>
						<Label className="visible-xs visible-sm">{i18n.t("common.data_source")}</Label><span><a role="button" onClick={this.handleSelectDataSource}>{this.state.media.data_source}</a></span>
					</Col>
					<Col md={2} sm={12} xs={12}>
						<Label className="visible-xs visible-sm">{i18n.t("common.artist")}</Label><span><a role="button" onClick={this.handleSelectArtist}>{artist}</a></span>
					</Col>
					<Col md={2} sm={12} xs={12}>
						<Label className="visible-xs visible-sm">{i18n.t("common.album")}</Label><span><a role="button" onClick={this.handleSelectAlbum}>{album}</a></span>
					</Col>
					<Col md={2} sm={12} xs={12}>
						<Label className="visible-xs visible-sm">{i18n.t("common.title")}</Label><a role="button" onClick={this.handleOpenModal}>{name}</a>
					</Col>
					<Col md={2} sm={12} xs={12}>
						<VisibilitySensor
							scrollCheck
							scrollThrottle={100}
							intervalDelay={8000}
							onChange={this.onChangeVisibility}
							minTopValue={10}
							partialVisibility={true}>
							<a role="button" onClick={this.handleOpenModal}>
								{cover}
							</a>
						</VisibilitySensor>
					</Col>
				</Row>
				<ModalMedia show={this.state.modalShow} media={this.state.media} title={this.state.modalTitle} />
			</div>
		);
	}
}

export default MediaRow;

