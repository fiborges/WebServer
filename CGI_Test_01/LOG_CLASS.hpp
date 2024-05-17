/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LOG_CLASS.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 11:15:40 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/17 11:40:39 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_CLASS_HPP
#define LOG_CLASS_HPP

#include <iostream>
#include <fstream>

class	LOG_CLASS
{
	private:
		std::string	fileName_;
		std::string	Content_;

	public:
		LOG_CLASS();
		~LOG_CLASS();

		static void	CreateLog(const char* fileName, std::string& Content);
};

#endif
