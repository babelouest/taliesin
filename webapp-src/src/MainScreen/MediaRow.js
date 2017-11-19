import React, { Component } from 'react';
import { Row, Col, Image } from 'react-bootstrap';
import StateStore from '../lib/StateStore';
import ModalMedia from '../Modal/ModalMedia';

class MediaRow extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {
      stream: props.stream,
			media: props.media, 
      index: props.index,
			date: props.date,
			imgThumbBlob: false,
			modalShow: false,
			modalTitle: this.buildTitle(props.media)
		};

		this.loadCover = this.loadCover.bind(this);
		this.handleOpenModal = this.handleOpenModal.bind(this);
		
		this.loadCover();
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
      stream: nextProps.stream,
			media: nextProps.media,
      index: nextProps.index,
			date: nextProps.date,
			imgThumbBlob: false,
			modalShow: false,
			modalTitle: this.buildTitle(nextProps.media)
		}, () => {
			this.loadCover();
		});
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
		if (this.state.media.data_source && this.state.media.path) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
			.then((result) => {
				this.setState({imgThumbBlob: result});
			})
			.fail(() => {
				this.setState({imgThumbBlob: false});
			});
		}
	}
	
  handleOpenModal() {
    this.setState({modalShow: true});
  }
  
	render() {
		var artist = "", album = "", name = "", date = "", cover = "";
		if (this.state.media.tags) {
			artist = this.state.media.tags.artist || this.state.media.tags.album_artist;
			album = this.state.media.tags.album;
			name = this.state.media.tags.title || this.state.media.name;
		} else {
			name = this.state.media.name;
		}
		if (this.state.date) {
			date = 
			<Col md={2}>
				{(new Date(this.state.date * 1000)).toLocaleString()}
			</Col>;
		} else {
			date = <Col md={1} />;
		}
		if (this.state.imgThumbBlob) {
			cover = <Image src={"data:image/jpeg;base64," + this.state.imgThumbBlob} thumbnail responsive className="cover-image-thumb"/>
		}
		return (
			<div>
				<Row>
					<Col md={12}>
						<hr/>
					</Col>
				</Row>
				<Row>
					{date}
					<Col md={2}>
						<span>{this.state.media.data_source}</span>
					</Col>
					<Col md={2}>
						<span>{artist}</span>
					</Col>
					<Col md={2}>
						<span>{album}</span>
					</Col>
					<Col md={2}>
						<a role="button" onClick={this.handleOpenModal}>{name}</a>
					</Col>
					<Col md={2}>
						<a role="button" onClick={this.handleOpenModal}>{cover}</a>
					</Col>
				</Row>
				<ModalMedia show={this.state.modalShow} media={this.state.media} title={this.state.modalTitle} />
			</div>
		);
	}
}

export default MediaRow;

