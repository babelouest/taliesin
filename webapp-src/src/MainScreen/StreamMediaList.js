import React, { Component } from 'react';
import { Row, Col, Label, ButtonGroup, Button } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import MediaRow from './MediaRow';
import i18n from '../lib/i18n';

class StreamMediaList extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {
			stream: props.stream,
			list: [],
			offset: 0,
			limit: 100,
			listLoaded: false
		};
		
		this.getMediaList();
		
		this.getMediaList = this.getMediaList.bind(this);
		this.handleMediaListPrevious = this.handleMediaListPrevious.bind(this);
		this.handleMediaListNext = this.handleMediaListNext.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			stream: nextProps.stream,
			list: [],
			offset: 0,
			limit: 100,
			listLoaded: false
		}, () => {
			this.getMediaList();
		});
	}
	
	getMediaList() {
		if (this.state.stream.name) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "list", parameters: {offset: this.state.offset, limit: this.state.limit}})
			.then((result) => {
				var list = [];
				result.forEach((media, index) => {
					list.push(<MediaRow stream={this.state.stream.webradio?false:this.state.stream.name} elements={this.state.stream.elements} media={media} index={index} key={index} />);
				});
				this.setState({list: list, listLoaded: true});
			});
		}
	}
	
	handleMediaListPrevious() {
		this.setState({offset: this.state.offset - this.state.limit, listLoaded: false}, () => {
			this.getMediaList();
		});
	}
	
	handleMediaListNext() {
		this.setState({offset: this.state.offset + this.state.limit, listLoaded: false}, () => {
			this.getMediaList();
		});
	}
	
  render() {
		return (
			<div>
				<Row>
					<Col md={4}>
						<ButtonGroup>
							<Button onClick={this.handleMediaListPrevious} disabled={!this.state.offset}>
								{i18n.t("common.previous_page")}
							</Button>
							<Button onClick={this.handleMediaListNext} disabled={(this.state.offset + this.state.list.length) >= this.state.stream.elements}>
								{i18n.t("common.next_page")}
							</Button>
						</ButtonGroup>
					</Col>
					<Col md={2} sm={6} xs={6}>
						<Label>{i18n.t("stream.total_media_files")}</Label>
					</Col>
					<Col md={2} sm={6} xs={6}>
						<span>{this.state.stream.elements}</span>
					</Col>
				</Row>
				<Row className="hidden-xs">
					<Col md={2}>
					</Col>
					<Col md={2}>
						<Label>{i18n.t("common.data_source")}</Label>
					</Col>
					<Col md={2}>
						<Label>{i18n.t("common.artist")}</Label>
					</Col>
					<Col md={2}>
						<Label>{i18n.t("common.album")}</Label>
					</Col>
					<Col md={2}>
						<Label>{i18n.t("common.title")}</Label>
					</Col>
					<Col md={2}>
						<Label>{i18n.t("common.cover")}</Label>
					</Col>
				</Row>
				{this.state.list}
				{this.state.listLoaded?"":<FontAwesome name="spinner" spin />}
			</div>
		);
	}
}

export default StreamMediaList;
