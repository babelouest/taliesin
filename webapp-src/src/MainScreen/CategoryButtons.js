import React, { Component } from 'react';
import { ButtonGroup, Button } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class CategoryButtons extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue
		};
		this.handleChangeCategory = this.handleChangeCategory.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue
		});
	}
	
	handleChangeCategory(category) {
		if (!this.state.categoryValue) {
			StateStore.dispatch({type: "setCurrentCategory", category: category});
		} else {
			StateStore.dispatch({type: "setCurrentCategory", category: this.state.category, categoryValue: this.state.categoryValue, subCategory: category});
		}
	}
	
	render() {
		var mediaButton, artistButton, albumButton, yearButton, genreButton;
		if (this.state.categoryValue && !this.state.subCategory) {
			mediaButton =
				<Button onClick={() => this.handleChangeCategory(false)}>
					Media
				</Button>
		}
		if (!this.state.subCategoryValue && this.state.subCategory !== "artist" && this.state.category !== "artist") {
			artistButton =
				<Button onClick={() => this.handleChangeCategory("artist")}>
					Artist
				</Button>
		}
		if (!this.state.subCategoryValue && this.state.subCategory !== "album" && this.state.category !== "album") {
			albumButton =
				<Button onClick={() => this.handleChangeCategory("album")}>
					Album
				</Button>
		}
		if (!this.state.subCategoryValue && this.state.subCategory !== "year" && this.state.category !== "year") {
			yearButton =
				<Button onClick={() => this.handleChangeCategory("year")}>
					Years
				</Button>
		}
		if (!this.state.subCategoryValue && this.state.subCategory !== "genre" && this.state.category !== "genre") {
			genreButton =
				<Button onClick={() => this.handleChangeCategory("genre")}>
					Genre
				</Button>
		}
		return (
			<ButtonGroup>
				{mediaButton}
				{artistButton}
				{albumButton}
				{yearButton}
				{genreButton}
			</ButtonGroup>
		);
	}
}

export default CategoryButtons;
