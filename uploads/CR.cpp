/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CR.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 14:51:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/11 10:46:53 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CR.hpp"

CR::CR() {}

CR::~CR() {}

CR::CR(int ClientSocket)
{
	this->ClientSocket_ = ClientSocket;
}

CR::CR(std::string Cleaned)
{
	this->CleanedRequest = Cleaned;
}

bool	CR::ItIsChunked(std::string& Request)
{
	std::string	ToFind = "Transfer-Encoding: chunked";
	size_t		HeadersFinal = Request.find("\r\n\r\n");

	if (Request.substr(0, HeadersFinal).find(ToFind) != std::string::npos)
		return (true);
	else
		return (false);
	return false;
}

bool	CR::TheRequestIsFinished()
{
	size_t	FirstPos = this->FullRequest_.find("\r\n\r\n");

	if (FirstPos == std::string::npos)
		return false;
	else if (this->FullRequest_.find("\r\n\r\n", FirstPos + 4) == std::string::npos)
		return false;
	return true;
}

std::string	CR::HandleRequest(std::string RequestFromBegin)
{
	char	buffer[4096];
	int		bytesRead;

	this->FullRequest_ = RequestFromBegin;
	if (TheRequestIsFinished() == true)
	{
		ProcessChunked();
		return (this->CleanedRequest);
	}

	while ((bytesRead = recv(this->ClientSocket_, buffer, 4095, 0)) > 0)
	{
		buffer[bytesRead] = '\0';
		//std::cout << "Chunked Received:\n" << buffer << std::endl;
		this->FullRequest_.append(buffer);
		if (TheRequestIsFinished() == true)
			break;
	}
	if (bytesRead < 0)
		std::cerr << "Error on recv\n";

	std::cout << "TOTAL REQUEST:\n" << this->FullRequest_ << std::endl;
	std::cout << "[FINISH]\n";
	ProcessChunked();
	return (this->CleanedRequest);
}

int	ConvertToDeci(std::string hex)
{
	int	x = 0;
	int	NuSize = 0;
	char	Hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	while (hex[NuSize])
		NuSize++;

	NuSize--;

	for (int i = 0; hex[i]; i++)
	{
		if (hex[i] >= 'a' && hex[i] <= 'z')
			hex[i] -= 32;
	}

	for (int i = 0; hex[i]; i++)
	{
		for (int j = 0; j <= 16; j++)
		{
			if (hex[i] == Hex[j])
			{
				x += (j * std::pow(16, NuSize));
				NuSize--;
				break;
			}
		}
	}

	return (x);
}

std::string	CR::ProcessChunked()
{
	size_t		PosBegChunk = this->FullRequest_.find("\r\n\r\n") + 4;
	std::string	SizeOfChunkInHex;
	int			SizeOfChunkInDec;

	this->CleanedRequest.append(this->FullRequest_, 0, PosBegChunk);

	//std::cout << "FirstPartOfCleanedRequest:\n" << this->CleanedRequest << "$\n";

	size_t	InicioDosCHUNKES = this->FullRequest_.find("\r\n", PosBegChunk) + 2;

	while (this->FullRequest_[InicioDosCHUNKES] != '\r')
		SizeOfChunkInHex += this->FullRequest_[InicioDosCHUNKES++];
	SizeOfChunkInDec = ConvertToDeci(SizeOfChunkInHex);


	InicioDosCHUNKES += 2;
	//std::cout << "SIZE:" << SizeOfChunkInDec << std::endl;
	while (SizeOfChunkInDec != 0)
	{
		std::string	NewPiece;

		while (this->FullRequest_[InicioDosCHUNKES] != '\r')
			NewPiece += this->FullRequest_[InicioDosCHUNKES++];

		this->CleanedRequest.append(NewPiece);

		InicioDosCHUNKES += 2;

		std::string	ChunkSizeHex;

		while (this->FullRequest_[InicioDosCHUNKES] != '\r')
			ChunkSizeHex += this->FullRequest_[InicioDosCHUNKES++];
		SizeOfChunkInDec = ConvertToDeci(ChunkSizeHex);

		//std::cout << "SIZE:" << SizeOfChunkInDec << std::endl;
		//std::cout << "NEWPIECE:" << NewPiece << std::endl;

		InicioDosCHUNKES += 2;
	}

	this->CleanedRequest += "\r\n\r\n";
	std::cout << "CleandedRequest:\n" << this->CleanedRequest;
	std::cout << "[FINISH]\n";

	return (this->CleanedRequest);
}

void	CR::CheckTheChunk()
{
	std::string	test;
	size_t	BodyStart = this->CleanedRequest.find("\r\n\r\n") + 4;

	while (this->CleanedRequest[BodyStart] != '\r')
		test += this->CleanedRequest[BodyStart++];

	std::cout << "TEST_STRING:" << test << "[FINISH]\n";

	if (test.empty())
		throw CR_ExceptionClass(400);
}

const char* CR::CR_ExceptionClass::what() const throw()
{
	return ("ALERT: CR DETECTED ERROR\n");
}
