import React, { Component } from 'react';
import { Button, Modal, Row, Col, Label, FormControl } from 'react-bootstrap';

import i18n from '../lib/i18n';

class ModalEditMPD extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			show: props.show,
			player: props.player || {name: "", description: "", host: "", port: 0, password: ""},
			add: props.add,
			onCloseCb: props.onCloseCb
		};
		
		this.close = this.close.bind(this);
		this.handleChangeName = this.handleChangeName.bind(this);
		this.handleChangeDescription = this.handleChangeDescription.bind(this);
		this.handleChangeHost = this.handleChangeHost.bind(this);
		this.handleChangePort = this.handleChangePort.bind(this);
		this.handleChangePassword = this.handleChangePassword.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show,
			player: nextProps.player || {name: "", description: "", host: "", port: 0, password: ""},
			add: nextProps.add,
			onCloseCb: nextProps.onCloseCb
		});
	}

	close(result, e) {
		if (e) {
			e.preventDefault();
		}
		if (result) {
			this.state.onCloseCb(true, this.state.add, this.state.player);
		} else {
			this.state.onCloseCb(false);
		}
	}
	
	handleChangeName(e) {
		var player = this.state.player;
		player.name = e.target.value;
		this.setState({player: player});
	}
	
	handleChangeDescription(e) {
		var player = this.state.player;
		player.description = e.target.value;
		this.setState({player: player});
	}
	
	handleChangeHost(e) {
		var player = this.state.player;
		player.host = e.target.value;
		this.setState({player: player});
	}
	
	handleChangePort(e) {
		var player = this.state.player;
		player.port = e.target.value;
		this.setState({player: player});
	}
	
	handleChangePassword(e) {
		var player = this.state.player;
		player.password = e.target.value;
		this.setState({player: player});
	}
	
	render() {
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{this.state.add?i18n.t("modal.add_player"):i18n.t("modal.edit_player")}</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.name")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="text"
									value={this.state.player.name}
									placeholder={i18n.t("common.name")}
									disabled={!this.state.add}
									onChange={this.handleChangeName}
								/>
							</Col>
						</Row>
						<hr/>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.description")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="text"
									value={this.state.player.description}
									placeholder={i18n.t("common.description")}
									onChange={this.handleChangeDescription}
								/>
							</Col>
						</Row>
						<hr/>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.mpd_host")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="text"
									value={this.state.player.host}
									placeholder={i18n.t("modal.mpd_host")}
									onChange={this.handleChangeHost}
								/>
							</Col>
						</Row>
						<hr/>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.mpd_port")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="number"
									min="0"
									value={this.state.player.port}
									placeholder={i18n.t("modal.mpd_port")}
									onChange={this.handleChangePort}
								/>
							</Col>
						</Row>
						<hr/>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.mpd_password")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="password"
									value={this.state.player.password}
									placeholder={i18n.t("modal.mpd_password")}
									onChange={this.handleChangePassword}
								/>
							</Col>
						</Row>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEditMPD;
